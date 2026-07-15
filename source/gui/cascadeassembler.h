#ifndef CASCADEASSEMBLER_H
#define CASCADEASSEMBLER_H

#include <cstdint>
#include <functional>
#include <vector>

#include <tally.h> // Event

class ion;

struct TrackVertex
{
    float x, y, z; // position [nm]
    float energy; // [eV]
    float t; // time [ns]
    int16_t rid; // recoil generation (0 = source ion)
    int16_t aid; // atom id
};
// 24 bytes, laid out to map directly onto the OpenGL vertex buffer
static_assert(sizeof(TrackVertex) == 24, "TrackVertex must stay 24 bytes");

struct Track
{
    std::vector<TrackVertex> verts;
};

struct Cascade
{
    std::vector<Track> tracks;
};

// Rebuilds cascades from the event stream (one source-ion history per cascade).
// No Qt or OpenGL, so it can be tested on its own. feed() runs on the sim thread.
class CascadeAssembler
{
public:
    typedef std::function<void(Cascade &&)> sink_t;

    static uint32_t eventMask();

    explicit CascadeAssembler(sink_t sink);

    void feed(Event ev, const ion &i);
    void flush(); // hand off the cascade under construction
    void reset(); // drop it and wait for the next boundary

private:
    void beginCascade();
    void beginTrack(const ion &i);
    void addVertex(const ion &i);
    void endTrack() { inTrack_ = false; }
    void handoff();

    sink_t sink_;
    Cascade current_;
    bool inCascade_{ false };
    bool inTrack_{ false };
    bool waitBoundary_{ true };
};

#endif // CASCADEASSEMBLER_H
