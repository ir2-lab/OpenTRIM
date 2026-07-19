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
    assembler_.flush();
}
