#ifndef MCDATAMODEL_H
#define MCDATAMODEL_H

#include <functional>

#include <tally.h>

#include <mcdriver.h>

#include "qdatabrowser.h"

class TallyDataSet : public AbstractDataSet
{
public:
    TallyDataSet(std::shared_ptr<mcdriver> d, const tally &t, const tally &dt,
                 tally::tally_t type);
    virtual ~TallyDataSet() { }

    bool hasErrors() const override { return true; }
    bool is_x_categorical(size_t d) const override { return (type_ == tally::cT) || (d == 0); }
    size_t get_x_categorical(size_t d, strvec_t &x) const override;

protected:
    std::shared_ptr<mcdriver> driver_;
    ArrayNDd data_, errors_;
    tally::tally_t type_;
    strvec_t arrayNames_, atomLabels_;

    size_t get_x(size_t d, size_t n, double *v) const override;
    size_t get_y(size_t d, const dim_t &i0, size_t n, double *v) const override;
    size_t get_dy(size_t d, const dim_t &i0, size_t n, double *v) const override;
};

class UTallyDataSet : public AbstractDataSet
{
public:
    UTallyDataSet(std::shared_ptr<mcdriver> d, const user_tally *t, const user_tally *dt);
    virtual ~UTallyDataSet() { }

    bool hasErrors() const override { return true; }

protected:
    std::shared_ptr<mcdriver> driver_;
    const user_tally *t_;
    const user_tally *dt_;
    ArrayNDd data_, errors_;
    strvec_t atomLabels_;

    size_t get_x(size_t d, size_t n, double *v) const override;
    size_t get_y(size_t d, const dim_t &i0, size_t n, double *v) const override;
    size_t get_dy(size_t d, const dim_t &i0, size_t n, double *v) const override;
};

// for scalar or mostly 1D text data
class TextDataSet : public AbstractDataSet
{
public:
    typedef std::function<void(const TextDataSet *, strvec_t &)> getter_t;

    TextDataSet(std::shared_ptr<mcdriver> d, const std::string &name, const std::string &desc,
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
    std::shared_ptr<mcdriver> driver() const { return driver_; }

protected:
    std::shared_ptr<mcdriver> driver_;
    strvec_t x_category_;
    getter_t getter_;
};

// for static tabular data
class NumericDataSet : public AbstractDataSet
{
public:
    NumericDataSet(const std::string &name, const std::string &desc, const dim_t &dim,
                    const strvec_t &dn = {}, const strvec_t &dd = {})
        : AbstractDataSet(name, dim), x_category(dim.size()), x(dim.size()), data(dim)
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

class McDataModel : public QDataModel
{
    Q_OBJECT
public:
    explicit McDataModel(std::shared_ptr<mcdriver> d, QObject *parent = nullptr);

private:
    std::shared_ptr<mcdriver> driver_;
};

#endif // MCDATAMODEL_H
