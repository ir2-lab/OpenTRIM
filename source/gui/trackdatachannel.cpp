#include "trackdatachannel.h"

TrackDataChannel::TrackDataChannel(QObject *parent)
    : QObject(parent), assembler_([this](Cascade &&c) { enqueue(std::move(c)); })
{
}

void TrackDataChannel::onEvent(Event ev, const ion &i, void *p)
{
    static_cast<TrackDataChannel *>(p)->assembler_.feed(ev, i);
}

void TrackDataChannel::enqueue(Cascade &&c)
{
    bool notify;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        notify = ready_.empty();
        ready_.push_back(std::make_shared<Cascade>(std::move(c)));
    }
    if (notify)
        emit cascadeReady();
}

std::vector<std::shared_ptr<Cascade>> TrackDataChannel::takeCascades()
{
    std::vector<std::shared_ptr<Cascade>> out;
    std::lock_guard<std::mutex> lock(mtx_);
    out.assign(ready_.begin(), ready_.end());
    ready_.clear();
    return out;
}

void TrackDataChannel::flush()
{
    assembler_.flush();
}
