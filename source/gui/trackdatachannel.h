#ifndef TRACKDATACHANNEL_H
#define TRACKDATACHANNEL_H

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <QObject>

#include "cascadeassembler.h"

// Carries ion track data from the simulation thread (thread-0 handler) to the
// GUI thread: finished cascades are queued and drained with takeCascades().
class TrackDataChannel : public QObject
{
    Q_OBJECT
public:
    static uint32_t eventMask() { return CascadeAssembler::eventMask(); }

    // the handler; p is a TrackDataChannel*
    static void onEvent(Event ev, const ion &i, void *p);

    explicit TrackDataChannel(QObject *parent = nullptr);

    std::vector<std::shared_ptr<const Cascade>> takeCascades();

    bool isCapturing() const { return capturing_.load(std::memory_order_relaxed); }

public slots:
    void setCapturing(bool on) { capturing_.store(on, std::memory_order_relaxed); }
    void flush(); // call after the run has stopped

signals:
    void cascadeReady();

private:
    void dispatch(Event ev, const ion &i); // simulation thread only
    void enqueue(Cascade &&c); // simulation thread only

    CascadeAssembler assembler_;
    bool active_{ false }; // simulation-thread view of capturing_

    std::atomic_bool capturing_{ false };

    std::mutex mtx_;
    std::deque<std::shared_ptr<const Cascade>> ready_;
};

#endif // TRACKDATACHANNEL_H
