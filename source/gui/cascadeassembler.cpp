#include "cascadeassembler.h"

#include <cassert>
#include <climits>

#include <ion.h>
#include <target.h>

uint32_t CascadeAssembler::eventMask()
{
    return static_cast<uint32_t>(Event::NewSourceIon) | static_cast<uint32_t>(Event::NewRecoil)
            | static_cast<uint32_t>(Event::Scattering) | static_cast<uint32_t>(Event::IonStop)
            | static_cast<uint32_t>(Event::IonExit) | static_cast<uint32_t>(Event::Replacement);
}

CascadeAssembler::CascadeAssembler(sink_t sink) : sink_(std::move(sink)) { }

void CascadeAssembler::feed(Event ev, const ion &i)
{
    switch (ev) {
    case Event::NewSourceIon:
        // a new source ion closes the previous history
        if (inCascade_)
            handoff();
        waitBoundary_ = false;
        beginCascade();
        beginTrack(i);
        break;

    case Event::NewRecoil:
        if (waitBoundary_ || !inCascade_)
            break;
        endTrack();
        beginTrack(i);
        break;

    case Event::Scattering:
        if (waitBoundary_ || !inTrack_)
            break;
        addVertex(i);
        break;

    case Event::IonStop:
    case Event::IonExit:
    case Event::Replacement:
        if (waitBoundary_ || !inTrack_)
            break;
        addVertex(i);
        endTrack();
        break;

    default:
        break;
    }
}

void CascadeAssembler::flush()
{
    if (inCascade_)
        handoff();
}

void CascadeAssembler::reset()
{
    current_ = Cascade();
    inCascade_ = false;
    inTrack_ = false;
    waitBoundary_ = true;
}

void CascadeAssembler::beginCascade()
{
    current_ = Cascade();
    inCascade_ = true;
    inTrack_ = false;
}

void CascadeAssembler::beginTrack(const ion &i)
{
    current_.tracks.emplace_back();
    inTrack_ = true;
    addVertex(i);
}

void CascadeAssembler::addVertex(const ion &i)
{
    if (!inTrack_ || current_.tracks.empty())
        return;

    const vector3 &r = i.pos();
    const atom *a = i.myAtom();
    assert(i.recoil_id() <= INT16_MAX && (!a || a->id() <= INT16_MAX));

    TrackVertex v;
    v.x = r.x();
    v.y = r.y();
    v.z = r.z();
    v.energy = static_cast<float>(i.erg());
    v.t = static_cast<float>(i.t());
    v.rid = static_cast<int16_t>(i.recoil_id());
    v.aid = static_cast<int16_t>(a ? a->id() : 0);

    current_.tracks.back().verts.push_back(v);
}

void CascadeAssembler::handoff()
{
    if (!current_.tracks.empty() && sink_)
        sink_(std::move(current_));
    current_ = Cascade();
    inCascade_ = false;
    inTrack_ = false;
}
