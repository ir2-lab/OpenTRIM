#include "event_stream.h"

#include "ion.h"
#include "target.h"

#include <filesystem>
#include <cstdio>
#include <unistd.h>

namespace fs = std::filesystem;

void pka_event::setNatoms(int n, const std::vector<std::string>& labels)
{
    natoms_ = n;
    buff_.resize(5+3*n);
    columnNames_.resize(buff_.size());
    columnDescriptions_.resize(buff_.size());
    columnNames_[0] = "hid"; columnDescriptions_[0] = "history id";
    columnNames_[1] = "pid"; columnDescriptions_[1] = "PKA species id";
    columnNames_[2] = "cid"; columnDescriptions_[2] = "cell id";
    columnNames_[3] = "E"; columnDescriptions_[3] = "PKA energy [eV]";
    columnNames_[4] = "Tdam"; columnDescriptions_[4] = "Damage energy[eV]";
    for(int i=0; i<n; i++) {
        int k = ofVac + i;
        columnNames_[k] = "V";
        columnNames_[k] += std::to_string(i+1); 
        columnDescriptions_[k] = "Vacancies of ";
        columnDescriptions_[k] += labels[i];
        k += n;
        columnNames_[k] = "R";
        columnNames_[k] += std::to_string(i+1); 
        columnDescriptions_[k] = "Replacements of ";
        columnDescriptions_[k] += labels[i];
        k += n;
        columnNames_[k] = "I";
        columnNames_[k] += std::to_string(i+1); 
        columnDescriptions_[k] = "Interstitials of ";
        columnDescriptions_[k] += labels[i];
    }
}

void pka_event::init(const ion* i)
{
    reset();
    buff_[ofIonId] = i->ion_id();
    buff_[ofAtomId] = i->myAtom()->id();
    buff_[ofCellId] = i->cellid();
    buff_[ofErg] = i->erg() + i->myAtom()->El(); // add lattice energy to recoil E=T-El -> T=E+El
}

int event_stream::open(const event& ev)
{
    close_();

    cols_ = ev.size();
    rows_ = 0;
    event_proto_ = event(ev);

    auto tmpdir = fs::temp_directory_path();
    std::string fname(tmpdir);
    fname += "/event_stream_XXXXXX";

    int fd = mkstemp(fname.data());
    if (fd == -1) return -1;

    fs_ = fdopen(fd, "w+");
    if (fs_ == NULL) {
        close(fd);
        return -1;
    }

    return 0;
}

void event_stream::close_()
{
    if (fs_) {
        std::fclose(fs_);
        fs_=NULL;
    }
}
void event_stream::write(const event* ev)
{
    if (is_open()) {
        std::fwrite(ev->data(),sizeof ev->data()[0],
                    ev->size(),fs_);
        rows_++;
    }
}

int event_stream::merge(event_stream& ev)
{
    if ((ev.cols() != cols()) ||
        !is_open() || !ev.is_open()) return -1;

    if (ev.rows()==0) return 0;

    ev.rewind();

    // local mem buffer ~1MB
    size_t n = (1 << 10);
    std::vector<float> buff(n*cols());

    // copy data in chunks
    size_t nrows = ev.rows(); // total rows to copy
    while (nrows) {
        size_t n1 = std::min(nrows,n); // # of rows to copy in this iter
        nrows -= n1;
        rows_ += n1;

        // read from other stream
        ev.read(buff.data(), n1);

        // write to this stream
        write(buff.data(), n1);
    }

    return 0;  // successfully terminated
}

void event_stream::rewind()
{
    if (is_open()) std::rewind(fs_);
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
    return is_open() ?
               std::fread(buff,sizeof(float),nevents*cols_,fs_) :
               0;
}

size_t event_stream::write(const float *buff, size_t nevents)
{
    return is_open() ?
               std::fwrite(buff,sizeof(float),nevents*cols_,fs_) :
               0;
}

exit_event::exit_event() :
event(ofEnd,
{"hid","iid","cid",
"E","x","y","z","nx","ny","nz"},
{"history id","ion species id","ion's cell id before exiting","ion energy [eV]",
"x position [nm]","y position [nm]","z position [nm]",
"x direction cosine","y direction cosine","z direction cosine"})
{}

void exit_event::set(const ion* i) //, int cellid)
{
    buff_[ofIonId] = i->ion_id();
    buff_[ofAtomId] = i->myAtom()->id();
    buff_[ofCellId] = i->prev_cellid();
    buff_[ofErg]   = i->erg();
    buff_[ofPos]   = i->pos().x();
    buff_[ofPos+1] = i->pos().y();
    buff_[ofPos+2] = i->pos().z();
    buff_[ofDir]   = i->dir().x();
    buff_[ofDir+1] = i->dir().y();
    buff_[ofDir+2] = i->dir().z();
}
