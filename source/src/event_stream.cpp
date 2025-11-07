#include "event_stream.h"

#include "ion.h"
#include "target.h"
#include "tally.h"

#include <filesystem>
#include <cstdio>
#include <unistd.h>

namespace fs = std::filesystem;

pka_buffer::pka_buffer() : event_buffer(static_cast<uint32_t>(Event::CascadeComplete)), natoms_(0)
{
}

void pka_buffer::calc_nrt(const ion &i, const material *m)
{
    // Make NRT damage estimations
    if (m) { // case NRT_average
        // NRT/LSS damage based on material / average Ed, Z, M
        Tdam_LSS_ = m->LSS_Tdam(recoilE());
        NRT_LSS_ = m->NRT(Tdam_LSS_);
        // NRT damage based on material with true Tdam
        NRT_ = m->NRT(Tdam());
    } else { // case NRT_element
        // NRT/LSS damage based on element
        const atom *z2 = i.myAtom();
        Tdam_LSS_ = z2->LSS_Tdam(recoilE());
        NRT_LSS_ = z2->NRT(Tdam_LSS_);
        // NRT damage based on element with true Tdam
        NRT_ = z2->NRT(Tdam());
    }
}

void pka_buffer::setNatoms(int n, const std::vector<std::string> &labels)
{
    natoms_ = n;
    buff_.resize(ofVac + atom_cols_ * n);
    columnNames_.resize(buff_.size());
    columnDescriptions_.resize(buff_.size());
    int k = ofIonId;
    columnNames_[k] = "hid";
    columnDescriptions_[k] = "history id";
    k = ofAtomId;
    columnNames_[k] = "pid";
    columnDescriptions_[k] = "PKA species id";
    k = ofPos;
    columnNames_[k] = "x";
    columnDescriptions_[k] = "PKA x position [nm]";
    k = ofPos + 1;
    columnNames_[k] = "y";
    columnDescriptions_[k] = "PKA y position [nm]";
    k = ofPos + 2;
    columnNames_[k] = "z";
    columnDescriptions_[k] = "PKA z position [nm]";
    k = ofErg;
    columnNames_[k] = "E";
    columnDescriptions_[k] = "PKA energy [eV]";
    k = ofTdam;
    columnNames_[k] = "Tdam";
    columnDescriptions_[k] = "Damage energy[eV]";

    for (int i = 0; i < n; i++) {
        int k = ofVac + i;
        columnNames_[k] = "V";
        columnNames_[k] += std::to_string(i + 1);
        columnDescriptions_[k] = "Vacancies of ";
        columnDescriptions_[k] += labels[i];
        k += n;
        columnNames_[k] = "I";
        columnNames_[k] += std::to_string(i + 1);
        columnDescriptions_[k] = "Interstitials of ";
        columnDescriptions_[k] += labels[i];
        k += n;
        columnNames_[k] = "ICR";
        columnNames_[k] += std::to_string(i + 1);
        columnDescriptions_[k] = "Intra-cascade recombinations of ";
        columnDescriptions_[k] += labels[i];
        k += n;
        columnNames_[k] = "Corr. ICR";
        columnNames_[k] += std::to_string(i + 1);
        columnDescriptions_[k] = "Corr. Intra-cascade recombinations of ";
        columnDescriptions_[k] += labels[i];
    }
}

void pka_buffer::init(const ion *i)
{
    reset();
    buff_[ofIonId] = i->ion_id();
    buff_[ofAtomId] = i->myAtom()->id();
    buff_[ofPos] = i->pos0().x();
    buff_[ofPos + 1] = i->pos0().y();
    buff_[ofPos + 2] = i->pos0().z();
    buff_[ofErg] = i->erg0();
}

int event_stream::open()
{
    close_();

    auto tmpdir = fs::temp_directory_path();
    fname_ = tmpdir.u8string();
    fname_ += "/event_stream_XXXXXX";

    int fd = mkstemp(fname_.data());
    if (fd == -1)
        return -1;

    fs_ = fdopen(fd, "w+");
    if (fs_ == NULL) {
        close(fd);
        fname_.clear();
        return -1;
    }

    return 0;
}

void event_stream::close_()
{
    if (fs_) {
        std::fclose(fs_);
        fs_ = NULL;
        std::remove(fname_.c_str());
        fname_.clear();
    }
}
void event_stream::write(const event_buffer *ev)
{
    if (is_open()) {
        std::fwrite(ev->data(), sizeof ev->data()[0], ev->size(), fs_);
        rows_++;
    }
}

int event_stream::merge(event_stream &ev)
{
    if ((ev.cols() != cols()) || !is_open() || !ev.is_open())
        return -1;

    if (ev.rows() == 0)
        return 0;

    ev.rewind();

    // local mem buffer ~1MB
    size_t n = (1 << 10);
    std::vector<float> buff(n * cols());

    // copy data in chunks
    size_t nrows = ev.rows(); // total rows to copy
    while (nrows) {
        size_t n1 = std::min(nrows, n); // # of rows to copy in this iter
        nrows -= n1;

        // read from other stream
        ev.read(buff.data(), n1);

        // write to this stream
        write(buff.data(), n1);
    }

    return 0; // successfully terminated
}

bool event_stream::set_event_prototype(const event_buffer &ev)
{
    close_();
    cols_ = ev.size();
    event_proto_ = event_buffer(ev);
    return true;
}

void event_stream::rewind()
{
    if (is_open())
        std::rewind(fs_);
}

void event_stream::clear()
{
    if (is_open()) {
        std::rewind(fs_);
        rows_ = 0;
    }
}

size_t event_stream::read(float *buff, size_t nevents)
{
    return is_open() ? std::fread(buff, sizeof(float), nevents * cols_, fs_) : 0;
}

size_t event_stream::write(const float *buff, size_t nevents)
{
    if (is_open()) {
        rows_ += nevents;
        return std::fwrite(buff, sizeof(float), nevents * cols_, fs_);
    }
    return 0;
}

exit_buffer::exit_buffer()
    : event_buffer(static_cast<uint32_t>(Event::IonExit), ofEnd,
                   { "hid", "iid", "cid", "E", "x", "y", "z", "nx", "ny", "nz" },
                   { "history id", "ion species id", "ion's cell id before exiting",
                     "ion energy [eV]", "x position [nm]", "y position [nm]", "z position [nm]",
                     "x direction cosine", "y direction cosine", "z direction cosine" })
{
}

void exit_buffer::set(const ion *i) //, int cellid)
{
    buff_[ofIonId] = i->ion_id();
    buff_[ofAtomId] = i->myAtom()->id();
    buff_[ofCellId] = i->prev_cellid();
    buff_[ofErg] = i->erg();
    buff_[ofPos] = i->pos().x();
    buff_[ofPos + 1] = i->pos().y();
    buff_[ofPos + 2] = i->pos().z();
    buff_[ofDir] = i->dir().x();
    buff_[ofDir + 1] = i->dir().y();
    buff_[ofDir + 2] = i->dir().z();
}

// { ofHid = 0, ofRid = 1, ofIid = 2, ofCid = 3, ofDid = 4, ofPos = 5, ofEnd = 8 }
damage_event_buffer::damage_event_buffer()
    : event_buffer(static_cast<uint32_t>(Event::IonStop) | static_cast<uint32_t>(Event::Vacancy),
                   ofEnd, { "hid", "rid", "iid", "did", "x", "y", "z" },
                   { "history id", "recoil id", "ion species id",
                     "defect type id 0: vacancy, 1: interstitial", "x position [nm]",
                     "y position [nm]", "z position [nm]" })
{
}

void damage_event_buffer::set(const ion &i)
{
    buff_[ofHid] = i.ion_id();
    buff_[ofRid] = i.recoil_id();
    buff_[ofIid] = i.myAtom()->id();
    buff_[ofDid] = i.type();
    buff_[ofPos] = i.pos().x();
    buff_[ofPos + 1] = i.pos().y();
    buff_[ofPos + 2] = i.pos().z();
}
