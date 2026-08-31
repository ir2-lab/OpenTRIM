#ifndef CASCADEASSEMBLER_H
#define CASCADEASSEMBLER_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <tally.h> // Event

class ion;

struct TrackVertex
{
    float x, y, z; // position [nm]
    float energy; // [eV]
    float t; // time [ps]
    int16_t rid; // recoil generation (0 = source ion)
    int16_t aid; // atom id
};
// 24 bytes, laid out to map directly onto the OpenGL vertex buffer
static_assert(sizeof(TrackVertex) == 24, "TrackVertex must stay 24 bytes");

// Contiguous layout for glMultiDrawArrays: all vertices in buff, plus the start
// index and length of each track. start_pos/length are int32_t (GLint/GLsizei).
struct Cascade
{
    uint64_t id{ 0 }; // source-ion history id
    std::vector<TrackVertex> buff;
    std::vector<int32_t> start_pos;
    std::vector<int32_t> length;
    float duration{ 0.f };
    std::array<float, 2> energy_range{ 0.f, 0.f }; // min, max
    uint32_t epoch{ 0 };
};

// Builds cascades from the event stream. feed() (sim thread) copies events into a
// fixed buffer; a consumer thread parses them into cascades, off the physics thread.
class CascadeAssembler
{
public:
    typedef std::function<void(Cascade &&)> sink_t;

    static uint32_t eventMask();

    explicit CascadeAssembler(sink_t sink);
    ~CascadeAssembler();

    // producer, simulation thread. Single producer only: install the handler on
    // exactly one thread (install_event_handler thread_no).
    void feed(Event ev, const ion &i);
    void flush(); // hand off the tail cascade; call after the run stopped

    void setCapturing(bool on) { capturing_.store(on, std::memory_order_relaxed); }
    bool isCapturing() const { return capturing_.load(std::memory_order_relaxed); }
    uint64_t dropped() const { return dropped_.load(std::memory_order_relaxed); }

    // per-axis wrap threshold [nm]; 1e30 disables an axis
    void setWrapThresholds(float tx, float ty, float tz);

    void setEnergyThreshold(float eV); // 0 = off
    void setGenCutoff(int g); // < 0 = off
    void setFilterEpoch(uint32_t e);

private:
    struct RawEvent
    {
        Event ev;
        uint64_t ion_id; // source-ion history id (becomes Cascade.id)
        TrackVertex v;
    };
    static_assert(sizeof(RawEvent) == 40, "RawEvent must stay 40 bytes");

    struct RawBuffer
    {
        std::vector<RawEvent> data; // fixed capacity, sized once at construction
        size_t count{ 0 };
        bool resyncBefore{ false }; // events were dropped before this buffer
        uint32_t epoch{ 0 };
    };

    // Fixed FIFO of buffer pointers (guarded by mtx_). Never allocates.
    struct PtrRing
    {
        RawBuffer *slot[8];
        int head{ 0 }, tail{ 0 }, n{ 0 };
        bool empty() const { return n == 0; }
        void push(RawBuffer *b)
        {
            slot[tail] = b;
            tail = (tail + 1) & 7;
            ++n;
        }
        RawBuffer *pop()
        {
            RawBuffer *b = slot[head];
            head = (head + 1) & 7;
            --n;
            return b;
        }
    };

    // ---- producer (simulation thread only) ----
    RawBuffer *acquireFree();
    void publishReady(RawBuffer *b);
    RawBuffer *cur_{ nullptr };
    bool sawGap_{ false }; // producer skipped/dropped events since the last buffer

    // ---- consumer thread only ----
    void consumeLoop();
    void processBuffer(RawBuffer *b);
    void applyEvent(const RawEvent &r);
    void beginCascade(const RawEvent &r);
    void beginTrack(const TrackVertex &v);
    void addVertex(const TrackVertex &v);
    void endTrack();
    void handoff();

    // WaitForIon: waiting for a NewSourceIon. InTrack: a track is open.
    // BetweenTracks: cascade open, no track open.
    enum class State { WaitForIon, InTrack, BetweenTracks };
    State state_{ State::WaitForIon };
    Cascade current_;
    int32_t track_start_{ 0 }; // first index of the open track in current_.buff
    sink_t sink_;
    float activeWrapThresh_[3]{ 1e30f, 1e30f, 1e30f }; // consumer snapshot, per buffer
    float activeEnergyThresh_{ 0.f };
    int activeGenCutoff_{ -1 };
    uint32_t activeEpoch_{ 0 };
    bool trackDropped_{ false };

    // ---- shared ----
    std::vector<std::unique_ptr<RawBuffer>> pool_;
    std::mutex mtx_;
    std::condition_variable cvReady_; // consumer waits for a full buffer or stop
    std::condition_variable cvFinalize_; // flush waits for the drained tail handoff
    PtrRing free_;
    PtrRing ready_;
    std::atomic<bool> stop_{ false };
    std::atomic<bool> capturing_{ false }; // pass-through until the view is active
    std::atomic<uint64_t> dropped_{ 0 };
    std::atomic<uint32_t> captureEpoch_{ 0 };
    float wrapThresh_[3]{ 1e30f, 1e30f, 1e30f }; // [nm] wrap-split config, written under mtx_
    float energyThresh_{ 0.f }; // [eV] limit config, written under mtx_
    int genCutoff_{ -1 };
    bool finalize_{ false };
    bool finalizeDone_{ false };
    bool finalizeDiscard_{ false }; // flush found capture off: drop the partial tail
    std::thread consumer_;
};

#endif // CASCADEASSEMBLER_H
