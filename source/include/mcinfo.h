#ifndef _MCINFO_H_
#define _MCINFO_H_

#include <functional>
#include <list>

#include "mcdriver.h"

class mcinfo;

/**
 * @brief Base class for the mcinfo tree nodes
 */
class mcinfo_node
{
    // Private constructor
    mcinfo_node(const std::string &name, mcinfo *parent, const std::string &desc = {})
        : name_(name), parent_(parent), desc_(desc)
    {
    }

public:
    virtual ~mcinfo_node() = default;

    // disable copy
    mcinfo_node(const mcinfo_node &) = delete;
    mcinfo_node &operator=(const mcinfo_node &) = delete;

    /// Returns the node's name
    const std::string &name() const noexcept { return name_; }
    /// Returns the node's description
    const std::string &description() const noexcept { return desc_; }
    /// Returns a pointer to the node's parent. The root node returns null.
    mcinfo *parent() noexcept { return parent_; }
    /// Returns a const pointer to the node's parent. The root node returns null.
    const mcinfo *parent() const noexcept { return parent_; }
    /// Returns a pointer to the root of the node tree
    mcinfo *root();
    /// Returns a const pointer to the root of the node tree
    const mcinfo *root() const;
    /// Returns the node's path in the form "/path/to/node"
    std::string path() const;
    /**
     * @brief Get a node pointer from a path
     *
     * The path is of the form "/path/to/node"
     *
     * If the path starts with a "/" it is considered absolute
     * and the search will start from the root node.
     *
     * Otherwise, the path is considered relative to the current node.
     *
     * If the path is ill formed or it does not correspond to a node
     * the function returns nullptr.
     *
     * @param path Node path in the form "/path/to/node"
     * @return Pointer to node or null
     */
    const mcinfo_node *path2node(const std::string &path) const;
    /// Returns true if this node is a group node
    virtual bool is_group() const noexcept = 0;

private:
    std::string name_;
    mcinfo *parent_; // non-owning; parent outlives child
    std::string desc_;

    friend class mcinfo;
    friend class mcinfo_data_node;
};

class mcinfo_data_node : public mcinfo_node
{
public:
    typedef std::vector<size_t> dim_t;
    enum data_t { string, json, real64, real32, uint64, tally_score };

    bool is_group() const noexcept override { return false; }
    data_t type() const noexcept { return type_; }
    dim_t dim() const
    {
        dim_t d;
        if (dim_getter_)
            dim_getter_(*this, d);
        return d;
    }
    size_t size() const
    {
        dim_t d = dim();
        if (d.empty())
            return 0;
        size_t sz = 1;
        for (const size_t &di : d)
            sz *= di;
        return sz;
    }

    bool get(std::vector<std::string> &s) const { return get_(string_getter_, s); }
    bool get(std::vector<double> &s) const { return get_(real64_getter_, s); }
    bool get(std::vector<float> &s) const { return get_(real32_getter_, s); }
    bool get(std::vector<uint64_t> &s) const { return get_(uint64_getter_, s); }
    bool get(std::vector<double> &s, std::vector<double> &ds) const;

private:
    typedef std::function<void(const mcinfo_data_node &, dim_t &)> dim_getter_t;
    typedef std::function<void(const mcinfo_data_node &, std::vector<std::string> &)>
            string_getter_t;
    typedef std::function<void(const mcinfo_data_node &, std::vector<double> &)> real64_getter_t;
    typedef std::function<void(const mcinfo_data_node &, std::vector<float> &)> real32_getter_t;
    typedef std::function<void(const mcinfo_data_node &, std::vector<uint64_t> &)> uint64_getter_t;

    data_t type_;
    dim_getter_t dim_getter_;
    string_getter_t string_getter_;
    real64_getter_t real64_getter_;
    real32_getter_t real32_getter_;
    uint64_getter_t uint64_getter_;
    int extra_idx0_{ -1 };
    int extra_idx1_{ -1 };

    template <class GetterFunc, class T>
    bool get_(GetterFunc &g, std::vector<T> &v) const
    {
        if (!g)
            return false;
        if (!dim_getter_)
            return false;
        size_t sz = size();
        if (sz == 0)
            return false;
        if (v.size() != sz)
            v.resize(sz);
        g(*this, v);
        return true;
    }

    mcinfo_data_node(const std::string &name, mcinfo *parent, const std::string &desc,
                     const dim_getter_t &dg, const string_getter_t &g, bool is_json = false,
                     int utally_idx = -1)
        : mcinfo_node(name, parent, desc),
          type_(is_json ? json : string),
          dim_getter_(dg),
          string_getter_(g),
          extra_idx1_(utally_idx)
    {
    }

    mcinfo_data_node(const std::string &name, mcinfo *parent, const std::string &desc,
                     const dim_getter_t &dg, const real64_getter_t &g, int utally_idx = -1)
        : mcinfo_node(name, parent, desc),
          type_(real64),
          dim_getter_(dg),
          real64_getter_(g),
          extra_idx1_(utally_idx)
    {
    }

    mcinfo_data_node(const std::string &name, mcinfo *parent, const std::string &desc,
                     const dim_getter_t &dg, const real32_getter_t &g, int utally_idx = -1)
        : mcinfo_node(name, parent, desc),
          type_(real32),
          dim_getter_(dg),
          real32_getter_(g),
          extra_idx1_(utally_idx)
    {
    }

    mcinfo_data_node(const std::string &name, mcinfo *parent, const std::string &desc,
                     const dim_getter_t &dg, const uint64_getter_t &g, int utally_idx = -1)
        : mcinfo_node(name, parent, desc),
          type_(uint64),
          dim_getter_(dg),
          uint64_getter_(g),
          extra_idx1_(utally_idx)
    {
    }

    mcinfo_data_node(const std::string &name, mcinfo *parent, const std::string &desc,
                     const dim_getter_t &dg, int tally_idx, int utally_idx = -1)
        : mcinfo_node(name, parent, desc),
          type_(tally_score),
          dim_getter_(dg),
          extra_idx0_(tally_idx),
          extra_idx1_(utally_idx)
    {
    }

    friend class mcinfo;
};

class mcinfo : public mcinfo_node
{
public:
    mcinfo(std::shared_ptr<mcdriver> d);

    static std::string info_spec();

    bool is_group() const noexcept override { return true; }

    mcinfo &add_group(const std::string &name, const std::string &desc)
    {
        mcinfo *p = new mcinfo(name, this, desc, driver_);
        auto &slot = insert(name, std::unique_ptr<mcinfo>(p));
        return static_cast<mcinfo &>(*slot);
    }

    template <class... Args>
    mcinfo_data_node &add_data(const std::string &name, const std::string &desc, Args &&...args)
    {
        mcinfo_data_node *p = new mcinfo_data_node(name, this, desc, std::forward<Args>(args)...);
        auto &slot = insert(name, std::unique_ptr<mcinfo_data_node>(p));
        return static_cast<mcinfo_data_node &>(*slot);
    }

    // --- lookup: throws if absent ---
    mcinfo_node &operator[](const std::string &name)
    {
        auto it = index_.find(name);
        if (it == index_.end())
            throw std::out_of_range("mcinfo: no node named '" + name + "'");
        return *((it->second)->get());
    }
    const mcinfo_node &operator[](const std::string &name) const
    {
        auto it = index_.find(name);
        if (it == index_.end())
            throw std::out_of_range("mcinfo: no node named '" + name + "'");
        return *((it->second)->get());
    }
    // --- at(key): returns null if absent ---
    mcinfo_node *at(const std::string &name)
    {
        auto it = index_.find(name);
        if (it == index_.end())
            return nullptr;
        return (it->second)->get();
    }
    const mcinfo_node *at(const std::string &name) const
    {
        auto it = index_.find(name);
        if (it == index_.end())
            return nullptr;
        return (it->second)->get();
    }

    std::vector<std::string> keys() const
    {
        std::vector<std::string> result;
        result.reserve(children_.size());
        for (const auto &i : index_)
            result.push_back(i.first); // entry.first is the name
        return result;
    }

    bool contains(const std::string &name) const { return index_.count(name) != 0; }
    std::size_t size() const noexcept { return children_.size(); }
    const mcdriver *driver() const { return driver_.get(); }

private:
    std::shared_ptr<mcdriver> driver_;
    // using entry = std::pair<const std::string, std::unique_ptr<mcinfo_node>>;
    using entry = std::unique_ptr<mcinfo_node>;
    std::list<entry> children_; // owns + orders
    std::unordered_map<std::string,
                       std::list<entry>::iterator> index_; // name -> slot

    mcinfo(const std::string &name, mcinfo *parent, const std::string &desc,
           std::shared_ptr<mcdriver> d)
        : mcinfo_node(name, parent, desc), driver_(d)
    {
    }

    std::unique_ptr<mcinfo_node> &insert(const std::string &name,
                                         std::unique_ptr<mcinfo_node> child)
    {
        if (index_.count(name))
            throw std::invalid_argument("mcinfo: duplicate name '" + name + "'");
        children_.emplace_back(std::move(child));
        auto last = std::prev(children_.end());
        index_.emplace(name, last);
        return *last;
    }
};

inline mcinfo *mcinfo_node::root()
{
    mcinfo *p = parent_;
    if (!p)
        return (mcinfo *)this;
    while (p->parent())
        p = p->parent();
    return p;
}
inline const mcinfo *mcinfo_node::root() const
{
    const mcinfo *p = parent_;
    if (!p)
        return (mcinfo *)this;
    while (p->parent())
        p = p->parent();
    return p;
}

inline std::string mcinfo_node::path() const
{
    const mcinfo_node *p = this;
    std::string s(name());
    while (p->parent()) {
        p = p->parent();
        s.insert(0, "/");
        s.insert(0, p->name());
    }
    return s;
}

inline const mcinfo_node *mcinfo_node::path2node(const std::string &path) const
{
    if (path.empty())
        return nullptr;

    // split the path
    std::string part;
    std::istringstream ss(path);
    std::vector<std::string> split_path;
    bool absolute = false;
    bool start = true;
    while (std::getline(ss, part, '/')) {
        if (part.empty()) {
            if (start) {
                absolute = true;
                start = false;
            } else
                return nullptr;
            continue;
        }
        start = false;
        split_path.push_back(part);
    }

    // get the starting group node
    const mcinfo *parent;
    if (absolute)
        parent = root();
    else {
        if (is_group())
            parent = (mcinfo *)this;
        else
            return nullptr;
    }

    // traverse intermediate groups
    for (int i = 0; i < (int)split_path.size() - 1; ++i) {
        const std::string &s = split_path[i];
        if (parent->contains(s) && (*parent)[s].is_group())
            parent = (mcinfo *)parent->at(s);
        else
            return nullptr;
    }

    // get the node
    return parent->at(split_path.back());
}

#endif
