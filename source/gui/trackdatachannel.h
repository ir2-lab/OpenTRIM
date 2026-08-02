#ifndef TRACKDATACHANNEL_H
#define TRACKDATACHANNEL_H

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <QObject>

#include "cascadeassembler.h"

// Carries track data from the sim thread to the GUI thread. Finished cascades are
// queued and cascadeReady() is emitted from the consumer thread (so B-2 must use a
// queued connection); the GUI thread drains with takeCascades().
class TrackDataChannel : public QObject
{
    Q_OBJECT
public:
    static uint32_t eventMask() { return CascadeAssembler::eventMask(); }

    // the handler; p is a TrackDataChannel*
    static void onEvent(Event ev, const ion &i, void *p);

    explicit TrackDataChannel(QObject *parent = nullptr);

    std::vector<std::shared_ptr<const Cascade>> takeCascades();

    bool isCapturing() const { return assembler_.isCapturing(); }
    void setWrapThresholds(float tx, float ty, float tz)
    {
        assembler_.setWrapThresholds(tx, ty, tz);
    }
    void setEnergyThreshold(float eV) { assembler_.setEnergyThreshold(eV); }
    void setGenCutoff(int g) { assembler_.setGenCutoff(g); }
    void setFilterEpoch(uint32_t e) { assembler_.setFilterEpoch(e); }

public slots:
    void setCapturing(bool on) { assembler_.setCapturing(on); }
    void flush(); // call after the run has stopped

signals:
    void cascadeReady();

private:
    void enqueue(Cascade &&c); // runs on the assembler's consumer thread

    std::mutex mtx_;
    std::deque<std::shared_ptr<const Cascade>> ready_;

    // declared last: its consumer thread is joined before ready_/mtx_ are destroyed
    CascadeAssembler assembler_;
};

#endif // TRACKDATACHANNEL_H
