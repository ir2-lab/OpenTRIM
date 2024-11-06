#include "event_stream.h"

#include "ion.h"
#include "target.h"

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

int event_stream::open(const std::string& fname, const event& ev)
{
    close();
    fname_ = fname;
    cols_ = ev.size();
    rows_ = 0;
    event_proto_ = event(ev);
    fs_.open(fname_, std::ios::binary);
    return fs_.is_open() ? 0 : -1;
}

void event_stream::close()
{
    if (fs_.is_open()) fs_.close();
}
void event_stream::write(const event* ev)
{
    if (fs_.is_open()) {
        fs_.write((char*)ev->data(),ev->size()*sizeof(float));
        rows_++;
    }
}

void event_stream::remove()
{
    if (!fname_.empty() && !is_open()) {
        std::remove(fileName().c_str());
        rows_ = 0;
    }
}

int event_stream::merge(const event_stream& ev)
{
    if ((ev.cols() != cols()) ||
        is_open() || ev.is_open()) return -1;

    if (ev.rows()==0) return 0;

    std::ifstream ifs(ev.fileName(), std::ios::binary);
    fs_.open(fname_, std::ios::out | std::ios::binary | std::ios::app );

    // mem buffer ~1MB
    size_t n = (1 << 10);
    std::vector<float> buff(n*cols());

    // copy data in chunks
    size_t nrows = ev.rows(); // total rows to copy
    while (nrows) {
        size_t n1 = std::min(nrows,n); // # of rows to copy in this iter
        nrows -= n1;
        rows_ += n1;
        size_t nbytes = n1*cols()*sizeof(float);

        // read from raw buffer
        ifs.read((char*)buff.data(), nbytes);

        // write to file
        fs_.write((char*)buff.data(), nbytes);
    }

    ifs.close();
    fs_.close();

    return 0;  // successfully terminated
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
