#ifndef MCPLOTINFO_H
#define MCPLOTINFO_H

#include <functional>

#include <tally.h>

#include "mcdriverobj.h"

#include "json_defs_p.h"

#include "qdatabrowser.h"

class TallyDataStore : public AbstractDataStore
{
public:
    TallyDataStore(const McDriverObj *d, const tally &t, const tally &dt, tally::tally_t type);
    virtual ~TallyDataStore() { }

    bool hasErrors() const override { return true; }
    bool is_x_categorical(size_t d) const override { return (type_ == tally::cT) || (d == 0); }
    size_t get_x_categorical(size_t d, strvec_t &x) const override;

protected:
    const McDriverObj *driver_{ nullptr };
    ArrayNDd data_, errors_;
    tally::tally_t type_;
    strvec_t arrayNames_, atomLabels_;

    size_t get_x(size_t d, size_t n, double *v) const override;
    size_t get_y(size_t d, const dim_t &i0, size_t n, double *v) const override;
};

struct mcplotinfo
{
    typedef std::vector<size_t> dim_t;
    enum info_t { group, string, json, real64, real32, uint64, tally_score };
    typedef nlohmann::ordered_map<std::string, mcplotinfo *> mcplotinfo_map_t;

    mcplotinfo() = default; // creates a group
    mcplotinfo(const mcplotinfo &o) = default;
    explicit mcplotinfo(const McDriverObj *d);
    mcplotinfo &operator=(const mcplotinfo &rhs);
    ~mcplotinfo()
    {
        for (auto &ch : children_)
            delete ch.second;
        if (data_)
            delete data_;
    }

    mcplotinfo *add_child(const std::string &key, mcplotinfo *m)
    {
        children_[key] = m;
        return m;
    }

    info_t type() const { return type_; }
    const mcplotinfo_map_t &children() const { return children_; }
    mcplotinfo_map_t &children() { return children_; }
    AbstractDataStore *takeData()
    {
        AbstractDataStore *d = data_;
        data_ = nullptr;
        return d;
    }

protected:
    info_t type_{ mcplotinfo::group };
    mcplotinfo_map_t children_;
    AbstractDataStore *data_{ nullptr };

    mcplotinfo(TallyDataStore *t) : type_(tally_score), data_(t) { }
};

inline mcplotinfo &mcplotinfo::operator=(const mcplotinfo &rhs)
{
    type_ = rhs.type_;
    children_.clear();
    for (const auto &ch : rhs.children_) {
        children_[ch.first] = ch.second;
    }
    data_ = rhs.data_;
    return *this;
}

#endif // MCPLOTINFO_H
