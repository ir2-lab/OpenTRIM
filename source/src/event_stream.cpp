#include "event_stream.h"

#include "ion.h"
#include "target.h"
#include "tally.h"
#include "cascade_queue.h"

#include <filesystem>
#include <cstdio>
#include <unistd.h>

namespace fs = std::filesystem;

void pka_event::mark(const tally &t)
{
    mark_T_ = 0.0f;
    int ncells = t.at(1).dim()[1];
    float *bv = mark_buff_.data();
    float *br = mark_buff_.data() + natoms_;
    float *bi = mark_buff_.data() + 2 * natoms_;
    for (int i = 0; i < natoms_; i++) {
        *bv = *br = *bi = 0.0f;
        const double *ps = &t.at(tally::eStored)(i + 1, 0);
        const double *pl = &t.at(tally::eLattice)(i + 1, 0);
        const double *pv = &t.at(tally::cV)(i + 1, 0);
        const double *pr = &t.at(tally::cR)(i + 1, 0);
        const double *pi = &t.at(tally::cI)(i + 1, 0);

        for (int j = 0; j < ncells; j++) {
            mark_T_ += *ps++ + *pl++;
            *bv += *pv++;
            *bi += *pi++;
            *br += *pr++;
        }
        bv++;
        br++;
        bi++;
    }
}

void pka_event::cascade_start(const ion &i)
{
    buff_[ofTdam] = mark_T_;

    float *p = buff_.data() + ofVac;
    float *pend = p + 3 * natoms_;
    const float *b = mark_buff_.data();
    while (p < pend) {
        *p = *b;
        p++;
        b++;
    }
}

void pka_event::cascade_end(const ion &i, const cascade_queue *cq)
{
    // get the damage energy =
    buff_[ofTdam] = mark_T_ - buff_[ofTdam]; // + i.myAtom()->El();
    float *p = buff_.data() + ofVac;
    float *pend = p + 3 * natoms_;
    const float *b = mark_buff_.data();
    while (p < pend) {
        *p = *b - *p;
        p++;
        b++;
    }
    // get the recombinations
    if (cq) {
        float *p = buff_.data() + ofVac + 3 * natoms_;
        cq->count_riv(p); // cq returns the count of recombined FPs
    }
    // store the Rp
    // vector3 r = i.pos() - i.pos0();
    // buff_[ofRp] = r.norm();
}

void pka_event::cascade_complete(const ion &i, tally &t, const material *m)
{
    // Make NRT damage estimations
    const atom *z2;
    std::array<float, 5> dp;
    dp.fill(0.f);
    dp[0] = recoilE();

    if (m) { // case NRT_average
        // NRT/LSS damage based on material / average Ed, Z, M
        dp[1] = m->LSS_Tdam(recoilE());
        dp[2] = m->NRT(dp[1]);
        // NRT damage based on material with true Tdam
        dp[3] = Tdam();
        dp[4] = m->NRT(dp[3]);
    } else { // case NRT_element
        // NRT/LSS damage based on element
        z2 = i.myAtom();
        dp[1] = z2->LSS_Tdam(recoilE());
        dp[2] = z2->NRT(dp[1]);
        // NRT damage based on element with true Tdam
        dp[3] = Tdam();
        dp[4] = z2->NRT(dp[3]);
    }

    t(Event::CascadeComplete, i, dp.data());
}

void pka_event::setNatoms(int n, const std::vector<std::string> &labels)
{
    natoms_ = n;
    buff_.resize(ofVac + 4 * n);
    mark_buff_.resize(3 * n);
    columnNames_.resize(buff_.size());
    columnDescriptions_.resize(buff_.size());
    int k = ofIonId;
    columnNames_[k] = "hid";
    columnDescriptions_[k] = "history id";
    k = ofAtomId;
    columnNames_[k] = "pid";
    columnDescriptions_[k] = "PKA species id";
    k = ofCellId;
    columnNames_[k] = "cid";
    columnDescriptions_[k] = "cell id";
    k = ofErg;
    columnNames_[k] = "E";
    columnDescriptions_[k] = "PKA energy [eV]";
    k = ofTdam;
    columnNames_[k] = "Tdam";
    columnDescriptions_[k] = "Damage energy[eV]";
    // k = ofRp;
    // columnNames_[k] = "Rp"; columnDescriptions_[k] = "I-V distance [nm]";
    for (int i = 0; i < n; i++) {
        int k = ofVac + i;
        columnNames_[k] = "V";
        columnNames_[k] += std::to_string(i + 1);
        columnDescriptions_[k] = "Vacancies of ";
        columnDescriptions_[k] += labels[i];
        k += n;
        columnNames_[k] = "R";
        columnNames_[k] += std::to_string(i + 1);
        columnDescriptions_[k] = "Replacements of ";
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
    }
}

void pka_event::init(const ion *i)
{
    reset();
    buff_[ofIonId] = i->ion_id();
    buff_[ofAtomId] = i->myAtom()->id();
    buff_[ofCellId] = i->cellid();
    buff_[ofErg] = i->erg() + i->myAtom()->El(); // add lattice energy to recoil E=T-El -> T=E+El
}

int event_stream::open(const event &ev)
{
    close_();

    cols_ = ev.size();
    rows_ = 0;
    event_proto_ = event(ev);

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
void event_stream::write(const event *ev)
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

exit_event::exit_event()
    : event(ofEnd, { "hid", "iid", "cid", "E", "x", "y", "z", "nx", "ny", "nz" },
            { "history id", "ion species id", "ion's cell id before exiting", "ion energy [eV]",
              "x position [nm]", "y position [nm]", "z position [nm]", "x direction cosine",
              "y direction cosine", "z direction cosine" })
{
}

void exit_event::set(const ion *i) //, int cellid)
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
