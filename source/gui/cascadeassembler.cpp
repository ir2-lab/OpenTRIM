#include "cascadeassembler.h"

#include <cassert>
#include <climits>
#include <cmath>

#include <ion.h>
#include <target.h>

// ~10 MB raw buffers (40 B/record), ~25 hand-offs/s at the ~230 MB/s peak. 4
// buffers give one buffer of slack so the consumer can jitter without dropping.
static const size_t kBufferRecords = 262144;
static const size_t kNumBuffers = 4;

// typical vertices per cascade; a larger one grows once (on the consumer thread)
static const size_t kCascadeReserve = 65536;

uint32_t CascadeAssembler::eventMask()
{
    return static_cast<uint32_t>(Event::NewSourceIon) | static_cast<uint32_t>(Event::NewRecoil)
            | static_cast<uint32_t>(Event::Scattering) | static_cast<uint32_t>(Event::IonStop)
            | static_cast<uint32_t>(Event::IonExit) | static_cast<uint32_t>(Event::Replacement);
}

CascadeAssembler::CascadeAssembler(sink_t sink) : sink_(std::move(sink))
{
    pool_.reserve(kNumBuffers);
    for (size_t k = 0; k < kNumBuffers; ++k) {
        std::unique_ptr<RawBuffer> b(new RawBuffer());
        b->data.resize(kBufferRecords); // the only per-buffer allocation, done once
        pool_.push_back(std::move(b));
    }
    cur_ = pool_[0].get();
    for (size_t k = 1; k < kNumBuffers; ++k)
        free_.push(pool_[k].get());

    consumer_ = std::thread(&CascadeAssembler::consumeLoop, this);
}

CascadeAssembler::~CascadeAssembler()
{
    {
        std::lock_guard<std::mutex> lk(mtx_); // set stop_ under the lock: no lost wakeup
        stop_.store(true, std::memory_order_relaxed);
    }
    cvReady_.notify_all();
    if (consumer_.joinable())
        consumer_.join();
}

void CascadeAssembler::setWrapThresholds(float tx, float ty, float tz)
{
    std::lock_guard<std::mutex> lk(mtx_);
    wrapThresh_[0] = tx;
    wrapThresh_[1] = ty;
    wrapThresh_[2] = tz;
}

// -------------------- producer (simulation thread) --------------------

CascadeAssembler::RawBuffer *CascadeAssembler::acquireFree()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (free_.empty())
        return nullptr;
    return free_.pop();
}

void CascadeAssembler::publishReady(RawBuffer *b)
{
    {
        std::lock_guard<std::mutex> lk(mtx_);
        ready_.push(b);
    }
    cvReady_.notify_one();
}

void CascadeAssembler::feed(Event ev, const ion &i)
{
    if (!capturing_.load(std::memory_order_relaxed)) {
        // publish the partial buffer so a resume starts on a fresh, resync'd buffer
        if (cur_ != nullptr && cur_->count > 0) {
            publishReady(cur_);
            cur_ = nullptr;
        }
        sawGap_ = true;
        return;
    }

    if (cur_ == nullptr) {
        cur_ = acquireFree();
        if (cur_ == nullptr) { // consumer behind: drop, resync at the next cascade
            dropped_.fetch_add(1, std::memory_order_relaxed);
            sawGap_ = true;
            return;
        }
        cur_->resyncBefore = sawGap_;
        sawGap_ = false;
    }

    // copy the event fields by index - no allocation
    assert(cur_->count < cur_->data.size());
    RawEvent &slot = cur_->data[cur_->count];
    slot.ev = ev;
    slot.ion_id = static_cast<uint64_t>(i.ion_id()); // ion_id() is int today; store 64-bit
    const vector3 &r = i.pos();
    const atom *a = i.myAtom();
    assert(i.recoil_id() <= INT16_MAX && (!a || a->id() <= INT16_MAX));
    slot.v.x = r.x();
    slot.v.y = r.y();
    slot.v.z = r.z();
    slot.v.energy = static_cast<float>(i.erg());
    slot.v.t = static_cast<float>(i.t());
    slot.v.rid = static_cast<int16_t>(i.recoil_id());
    slot.v.aid = static_cast<int16_t>(a ? a->id() : 0);
    ++cur_->count;

    if (cur_->count == cur_->data.size()) {
        publishReady(cur_);
        cur_ = acquireFree();
        if (cur_ != nullptr) {
            cur_->resyncBefore = sawGap_;
            sawGap_ = false;
        } else {
            sawGap_ = true;
        }
    }
}

void CascadeAssembler::flush()
{
    // publish the partial buffer (or return it empty), then let the consumer
    // drain and close the tail cascade. Called after the sim thread has stopped.
    if (cur_ != nullptr) {
        if (cur_->count > 0) {
            cur_->resyncBefore = sawGap_;
            publishReady(cur_);
        } else {
            std::lock_guard<std::mutex> lk(mtx_);
            free_.push(cur_);
        }
        cur_ = nullptr;
        sawGap_ = false;
    }

    std::unique_lock<std::mutex> lk(mtx_);
    finalizeDone_ = false;
    // if capture is off the tail belongs to an interrupted window: drop it
    finalizeDiscard_ = !capturing_.load(std::memory_order_relaxed);
    finalize_ = true;
    cvReady_.notify_one();
    cvFinalize_.wait(lk, [this] { return finalizeDone_; });
}

// -------------------- consumer thread --------------------

void CascadeAssembler::consumeLoop()
{
    for (;;) {
        RawBuffer *b = nullptr;
        bool doFinalize = false;
        bool discard = false;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cvReady_.wait(lk, [this] {
                return stop_.load(std::memory_order_relaxed) || !ready_.empty() || finalize_;
            });
            if (!ready_.empty()) {
                b = ready_.pop(); // process buffers before honoring finalize/stop
            } else if (finalize_) {
                finalize_ = false;
                doFinalize = true;
                discard = finalizeDiscard_;
            } else {
                return; // stop_ and nothing left
            }
        }

        if (b != nullptr) {
            processBuffer(b);
            std::lock_guard<std::mutex> lk(mtx_);
            b->count = 0;
            b->resyncBefore = false;
            free_.push(b);
        } else if (doFinalize) {
            if (discard) {
                current_ = Cascade(); // capture was off: drop the interrupted cascade
            } else {
                if (state_ == State::InTrack)
                    endTrack();
                if (state_ == State::InTrack || state_ == State::BetweenTracks)
                    handoff();
            }
            state_ = State::WaitForIon; // re-arm for a possible next run
            {
                std::lock_guard<std::mutex> lk(mtx_);
                finalizeDone_ = true;
            }
            cvFinalize_.notify_all();
        }
    }
}

void CascadeAssembler::processBuffer(RawBuffer *b)
{
    { // snapshot once per buffer
        std::lock_guard<std::mutex> lk(mtx_);
        activeWrapThresh_[0] = wrapThresh_[0];
        activeWrapThresh_[1] = wrapThresh_[1];
        activeWrapThresh_[2] = wrapThresh_[2];
    }
    if (b->resyncBefore) { // a gap in the stream: drop the partial cascade
        current_ = Cascade();
        track_start_ = 0;
        state_ = State::WaitForIon;
    }
    for (size_t k = 0; k < b->count; ++k)
        applyEvent(b->data[k]);
}

void CascadeAssembler::applyEvent(const RawEvent &r)
{
    switch (state_) {
    case State::WaitForIon:
        if (r.ev == Event::NewSourceIon) {
            beginCascade(r.ion_id);
            beginTrack(r.v);
            state_ = State::InTrack;
        }
        break;

    case State::InTrack:
        switch (r.ev) {
        case Event::NewSourceIon: // fires once per source ion (core source flag)
            endTrack();
            handoff();
            beginCascade(r.ion_id);
            beginTrack(r.v);
            break;
        case Event::NewRecoil:
            endTrack();
            beginTrack(r.v);
            break;
        case Event::Scattering:
            addVertex(r.v);
            break;
        case Event::IonStop:
        case Event::IonExit:
        case Event::Replacement:
            addVertex(r.v);
            endTrack();
            state_ = State::BetweenTracks;
            break;
        default:
            break;
        }
        break;

    case State::BetweenTracks:
        switch (r.ev) {
        case Event::NewSourceIon:
            handoff();
            beginCascade(r.ion_id);
            beginTrack(r.v);
            state_ = State::InTrack;
            break;
        case Event::NewRecoil:
            beginTrack(r.v);
            state_ = State::InTrack;
            break;
        default: // stray scattering or a deferred end with no open track
            break;
        }
        break;
    }
}

void CascadeAssembler::beginCascade(uint64_t id)
{
    current_ = Cascade();
    current_.id = id;
    current_.duration = 0.0f;
    track_start_ = 0;
    current_.buff.reserve(kCascadeReserve);
}

void CascadeAssembler::beginTrack(const TrackVertex &v)
{
    track_start_ = static_cast<int32_t>(current_.buff.size());
    addVertex(v);
}

void CascadeAssembler::addVertex(const TrackVertex &v)
{
    // drop the segment crossing a periodic boundary
    if (static_cast<int32_t>(current_.buff.size()) > track_start_) {
        const TrackVertex &p = current_.buff.back();
        if (std::abs(p.x - v.x) > activeWrapThresh_[0] || std::abs(p.y - v.y) > activeWrapThresh_[1]
            || std::abs(p.z - v.z) > activeWrapThresh_[2]) {
            endTrack();
            track_start_ = static_cast<int32_t>(current_.buff.size());
        }
    }
    current_.buff.push_back(v);
}

void CascadeAssembler::endTrack()
{
    int32_t len = static_cast<int32_t>(current_.buff.size()) - track_start_;
    if (len > 0) {
        current_.start_pos.push_back(track_start_);
        current_.length.push_back(len);
        current_.duration = std::max(current_.duration, current_.buff.back().t);
    }
}

void CascadeAssembler::handoff()
{
    if (!current_.buff.empty() && sink_)
        sink_(std::move(current_));
    current_ = Cascade();
    track_start_ = 0;
}
