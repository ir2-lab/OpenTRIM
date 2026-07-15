#include "trackdatachannel.h"

TrackDataChannel::TrackDataChannel(QObject *parent)
    : QObject(parent), assembler_([this](Cascade &&c) { enqueue(std::move(c)); })
{
}

void TrackDataChannel::onEvent(Event ev, const ion &i, void *p)
{
    static_cast<TrackDataChannel *>(p)->dispatch(ev, i);
}

void TrackDataChannel::dispatch(Event ev, const ion &i)
{
    // reset on any change of the capture flag, so we start on a clean boundary
    const bool want = capturing_.load(std::memory_order_relaxed);
    if (want != active_) {
        active_ = want;
        assembler_.reset();
    }
    if (active_)
        assembler_.feed(ev, i);
}

void TrackDataChannel::enqueue(Cascade &&c)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        ready_.push_back(std::make_shared<const Cascade>(std::move(c)));
    }
    emit cascadeReady();
}

std::vector<std::shared_ptr<const Cascade>> TrackDataChannel::takeCascades()
{
    std::vector<std::shared_ptr<const Cascade>> out;
    std::lock_guard<std::mutex> lock(mtx_);
    out.assign(ready_.begin(), ready_.end());
    ready_.clear();
    return out;
}

void TrackDataChannel::flush()
{
    // called after the run stops, so the assembler is no longer in use
    assembler_.flush();
}
