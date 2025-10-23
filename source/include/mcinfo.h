#ifndef _MCINFO_H_
#define _MCINFO_H_

#include <functional>

#include "mcdriver.h"

#include "json_defs_p.h"

struct mcinfo
{
    typedef std::vector<size_t> dim_t;
    enum info_t { group, string, json, real64, real32, uint64, tally_score, invalid };
    typedef nlohmann::ordered_map<std::string, mcinfo> mcinfo_map_t;

    mcinfo &operator=(const mcinfo &rhs)
    {
        driver_ = rhs.driver();
        type_ = rhs.type_;
        description_ = rhs.description_;
        extra_idx0_ = rhs.extra_idx0_;
        extra_idx1_ = rhs.extra_idx1_;

        children_.clear();
        for (const auto &ch : rhs.children_) {
            children_[ch.first] = ch.second;
        }

        string_getter_ = rhs.string_getter_;
        real64_getter_ = rhs.real64_getter_;
        real32_getter_ = rhs.real32_getter_;
        uint64_getter_ = rhs.uint64_getter_;
        return *this;
    }

    mcinfo() = default;
    mcinfo(const mcinfo &o) = default;
    mcinfo(const mcdriver *d);

    mcinfo &operator[](const std::string &key) { return children_[key]; }

    info_t type() const { return type_; }
    const std::string &description() const { return description_; }
    // const dim_t &dim() const { return dim_; }
    const mcdriver *driver() const { return driver_; }
    // bool is_scalar() const { return dim_.size() == 1 && dim_[0] == 1; }
    const mcinfo_map_t &children() const { return children_; }

    bool get(std::vector<std::string> &s, dim_t &dim) const { return get_(string_getter_, s, dim); }
    bool get(std::vector<double> &s, dim_t &dim) const { return get_(real64_getter_, s, dim); }
    bool get(std::vector<float> &s, dim_t &dim) const { return get_(real32_getter_, s, dim); }
    bool get(std::vector<uint64_t> &s, dim_t &dim) const { return get_(uint64_getter_, s, dim); }
    bool get(std::vector<double> &s, std::vector<double> &ds, dim_t &dim) const;

private:
    typedef std::function<void(const mcinfo &, std::vector<std::string> &, dim_t &)>
            string_getter_t;
    typedef std::function<void(const mcinfo &, std::vector<double> &, dim_t &)> real64_getter_t;
    typedef std::function<void(const mcinfo &, std::vector<float> &, dim_t &)> real32_getter_t;
    typedef std::function<void(const mcinfo &, std::vector<uint64_t> &, dim_t &)> uint64_getter_t;

    const mcdriver *driver_{ nullptr };
    info_t type_{ mcinfo::invalid };
    std::string description_;
    // dim_t dim_{ 1 };
    int extra_idx0_{ -1 };
    int extra_idx1_{ -1 };
    mcinfo_map_t children_;
    string_getter_t string_getter_;
    real64_getter_t real64_getter_;
    real32_getter_t real32_getter_;
    uint64_getter_t uint64_getter_;

    mcinfo(const mcdriver *d, info_t t, const std::string &desc = {})
        : driver_(d), type_(t), description_(desc)
    {
    }

    mcinfo(const mcdriver *d, info_t t, const std::string &desc, const string_getter_t &g,
           int utally_idx = -1)
        : driver_(d), type_(t), description_(desc), string_getter_(g), extra_idx1_(utally_idx)
    {
    }

    mcinfo(const mcdriver *d, const std::string &desc, const real64_getter_t &g,
           int utally_idx = -1)
        : driver_(d), type_(real64), description_(desc), real64_getter_(g), extra_idx1_(utally_idx)
    {
    }

    mcinfo(const mcdriver *d, const std::string &desc, const real32_getter_t &g,
           int utally_idx = -1)
        : driver_(d), type_(real32), description_(desc), real32_getter_(g), extra_idx1_(utally_idx)
    {
    }

    mcinfo(const mcdriver *d, const std::string &desc, const uint64_getter_t &g,
           int utally_idx = -1)
        : driver_(d), type_(uint64), description_(desc), uint64_getter_(g), extra_idx1_(utally_idx)
    {
    }

    mcinfo(const mcdriver *d, const std::string &desc, int tally_idx, int utally_idx = -1)
        : driver_(d),
          type_(tally_score),
          description_(desc),
          extra_idx0_(tally_idx),
          extra_idx1_(utally_idx)
    {
    }

    template <class F, class T>
    bool get_(F &g, std::vector<T> &v, dim_t &d) const
    {
        if (!g)
            return false;
        g(*this, v, d);
        return true;
    }
};

#endif
