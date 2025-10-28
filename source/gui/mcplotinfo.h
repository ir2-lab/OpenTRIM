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
    size_t get_dy(size_t d, const dim_t &i0, size_t n, double *v) const override;
};

class UTallyDataStore : public AbstractDataStore
{
public:
    UTallyDataStore(const McDriverObj *d, const user_tally *t, const user_tally *dt);
    virtual ~UTallyDataStore() { }

    bool hasErrors() const override { return true; }

protected:
    const McDriverObj *driver_{ nullptr };
    const user_tally *t_;
    const user_tally *dt_;
    ArrayNDd data_, errors_;
    strvec_t atomLabels_;

    size_t get_x(size_t d, size_t n, double *v) const override;
    size_t get_y(size_t d, const dim_t &i0, size_t n, double *v) const override;
    size_t get_dy(size_t d, const dim_t &i0, size_t n, double *v) const override;
};

// for scalar or mostly 1D text data
class TextDataStore : public AbstractDataStore
{
public:
    typedef std::function<void(const TextDataStore *, strvec_t &)> getter_t;

    TextDataStore(const McDriverObj *d, const std::string &name, const std::string &desc,
                  size_t dim, const getter_t &g, const strvec_t &x_category = strvec_t());

    bool is_numeric() const override { return false; }
    bool is_x_categorical(size_t) const override { return !x_category_.empty(); }
    size_t get_x_categorical(size_t, strvec_t &x) const override
    {
        x = x_category_;
        return x.size();
    }
    size_t get_y_text(size_t d, const dim_t &i0, strvec_t &v) const override
    {
        getter_(this, v);
        return v.size();
    }
    const McDriverObj *driver() const { return driver_; }

protected:
    const McDriverObj *driver_{ nullptr };
    strvec_t x_category_;
    getter_t getter_;
};

// for static tabular data
class NumericDataStore : public AbstractDataStore
{
public:
    NumericDataStore(const std::string &name, const std::string &desc, const dim_t &dim,
                     const strvec_t &dn = {}, const strvec_t &dd = {})
        : AbstractDataStore(name, dim), x_category(dim.size()), x(dim.size()), data(dim)
    {
        desc_ = desc;
        dim_name_ = dn;
        dim_desc_ = dd;
    }

    bool is_x_categorical(size_t d) const override { return !x_category[d].empty(); }
    size_t get_x_categorical(size_t d, strvec_t &v) const override
    {
        v = x_category[d];
        return v.size();
    }

    std::vector<strvec_t> x_category;
    std::vector<vec_t> x;
    ArrayNDd data;

protected:
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
    mcplotinfo(UTallyDataStore *t) : type_(tally_score), data_(t) { }
    mcplotinfo(TextDataStore *t) : type_(string), data_(t) { }
    mcplotinfo(NumericDataStore *t) : type_(real64), data_(t) { }
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
