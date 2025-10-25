#include "mcplotinfo.h"

#include <mccore.h>

mcplotinfo::mcplotinfo(const McDriverObj *d)
{
    // group tally
    mcplotinfo *p = add_child("tally", new mcplotinfo);
    {
        std::map<std::string, std::map<std::string, int>> tmap;
        int k = 0;
        while (k < tally::std_tallies) {
            tmap[tally::arrayGroup(k)][tally::arrayName(k)] = k;
            k++;
        }

        for (const auto &tgroup : tmap) {
            mcplotinfo *p1 = p->add_child(tgroup.first, new mcplotinfo);
            for (const auto &tname : tgroup.second) {
                int it = tname.second;
                p1->add_child(tname.first,
                              new mcplotinfo(new TallyDataStore(d, d->getSim()->getTally(),
                                                                d->getSim()->getTallyVar(),
                                                                static_cast<tally::tally_t>(it))));
            }
        }
    }
}

TallyDataStore::TallyDataStore(const McDriverObj *d, const tally &t, const tally &dt,
                               tally::tally_t type)
    : driver_(d), data_(t.at(type)), errors_(dt.at(type)), type_(type)
{
    dim_ = data_.dim();
    name_ = t.arrayName(type);
    desc_ = t.arrayDescription(type);
    arrayNames_ = t.arrayNames();
    atomLabels_ = d->getSim()->getTarget().atom_labels();

    if (type == tally::cT) {
        dim_name_.push_back("tally_id");
        dim_name_.push_back("atom_id");
        dim_desc_.push_back("id of tally table");
        dim_desc_.push_back("id of atom");
    } else {
        dim_name_.push_back("atom_id");
        dim_name_.push_back("cell_id");
        dim_desc_.push_back("id of atom");
        dim_desc_.push_back("id of simulation cell");
    }
}

size_t TallyDataStore::get_x_categorical(size_t d, strvec_t &x) const
{
    if (type_ == tally::cT) {
        if (d == 0) {
            x = arrayNames_;
            return x.size();
        } else {
            x = atomLabels_;
            return x.size();
        }
    } else {
        if (d == 0) {
            x = atomLabels_;
            return x.size();
        } else {
            return 0;
        }
    }
}

size_t TallyDataStore::get_x(size_t d, size_t n, double *v) const
{
    return AbstractDataStore::get_x(d, n, v);
}

size_t TallyDataStore::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
{
    if (data_.isNull())
        return 0;
    size_t N = driver_->getSim()->ion_count();
    size_t m = data_.dim()[d]; // size of dimension d
    size_t s = data_.stride()[d]; // stride of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = &data_(i0); // pointer to 1st data value
    const double *vend = v + m;
    for (; v < vend; ++v, p += s)
        *v = *p / N;
    return m;
}
