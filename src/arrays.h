#ifndef ARRAYS_H
#define ARRAYS_H

#include <memory>
#include <cstring>
#include <cassert>
#include <vector>
#include <cstring>

/**
 * @brief A N-dimensional explicitly shared array class
 * 
 * This class is used for storing tally and other numeric 
 * or other table data
 * that may be shared read-only among the threads of execution.
 * 
 * Explicitly shared means that in the following code
 * @code {.cpp}
 * ArrayND<double> A(2,3,4);
 * ArrayND<double> B = A;
 * @endcode
 * the objects A and B point to the same 
 * array in memory.
 * 
 * The data are stored in C-style row-major order.
 * 
 * @tparam Scalar The type of the array element
 * 
 * @ingroup Tallies
 */
template<typename Scalar>
class ArrayND
{
    typedef ArrayND<Scalar> _Self;

    struct Private
    {
        std::vector<size_t> dim;
        std::vector<Scalar> buffer;

        static size_t calc_size(const std::vector<size_t>& d)
        {
            size_t S(1);
            for(auto s : d) S *= s;
            return S;
        }

        Private() : dim(), buffer()
        {}

        explicit Private(size_t i) :
            dim(1),
            buffer(i,Scalar(0))
        {
            dim[0] = i;
        }

        Private(size_t i, size_t j) :
            dim(2),
            buffer(i*j,Scalar(0))
        {
            dim[0] = i; dim[1] = j;
        }

        Private(size_t i, size_t j, size_t k) :
            dim(3),
            buffer(i*j*k,Scalar(0))
        {
            dim[0] = i; dim[1] = j; dim[2] = k;
        }

        Private(const std::vector<size_t>& d) :
            dim(d), buffer(calc_size(d),Scalar(0))
        {}

        Private(const Private& p) : dim(p.dim), buffer(p.buffer)
        {}

        void resize(const std::vector<size_t>& d) {
            dim = d;
            buffer.resize(calc_size(d));
        }

        size_t idx(int i, int j) {
            return (dim.size() == 2) ? i*dim[1] + j : 0;
        }
        size_t idx(int i, int j, int k) {
            return (dim.size() == 3) ? (i*dim[1] + j)*dim[2] + k : 0;
        }
        size_t idx(const std::vector<size_t>& d) {
            assert(dim.size() == d.size());
            size_t k = d[0];
            for(int i = 1; i<d.size(); i++)  k = k*dim[i] + d[i];
            return k;
        }
        void copyTo(Private& other) {
            std::memcpy(other.buffer.data(),
                        buffer.data(),
                        buffer.size()*sizeof(Scalar));
        }
    };

    std::shared_ptr<Private> P_;

    ArrayND(const Private& p) :
        P_(new Private(p))
    {}

public:
    /// Constructs an empty array
    ArrayND() = default;

    /// Constructs an N-dimensional array of dimensions defined by dim
    explicit ArrayND(const std::vector<size_t>& dim) :
        P_(new Private(dim))
    {}
    /// Constructs a 1D array of dimension i
    explicit ArrayND(int i) :
        P_(new Private(i))
    {}
    /// Constructs a 2D array of dimensions i x j
    explicit ArrayND(int i, int j) :
        P_(new Private(i,j))
    {}
    /// Constructs a 3D array of dimensions i x j x k
    explicit ArrayND(int i, int j, int k) :
        P_(new Private(i,j,k))
    {}
    /// Copy constructor. Data is shared. 
    ArrayND(const ArrayND& other) : P_(other.P_)
    {}
    /// Assignement operator. Data is shared.
    ArrayND& operator=(const ArrayND& other)
    { P_ = other.P_; return *this; }
    /// Creates a copy of the array in memory and returns the new array
    ArrayND copy() const
    {
        return P_.get() ? ArrayND(*P_) : ArrayND();
    }
    /// Creates a copy of the array in memory and returns the new array
    void copyTo(ArrayND& other) const
    {
        if (P_ && other.P_ && size()==other.size()) P_->copyTo(*(other.P_));
    }
    /// Returns true is the array is empty
    bool isNull() const { return P_ == nullptr; }
    /// Returns the dimensions of the array
    const std::vector<size_t>& dim() const { return P_->dim; }
    /// Returns the total number of elements 
    size_t size() const { return P_->buffer.size(); }
    /// Returns the number of dimensions
    int ndim() const { return P_->dim.size(); }

    /// Returns the i-th element in C-style operator
    Scalar& operator[](int i) { return P_->buffer[i]; }
    const Scalar& operator[](int i) const { return P_->buffer[i]; }
    /// Returns the i-th element with () operator, i is zero based
    Scalar& operator()(int i) { return P_->buffer[i]; }
    const Scalar& operator()(int i) const { return P_->buffer[i]; }
    /// Returns the (i,j) element with () operator, i,j are zero based, row-major storage layout
    Scalar& operator()(int i, int j) { return P_->buffer[P_->idx(i,j)]; }
    const Scalar& operator()(int i, int j) const { return P_->buffer[P_->idx(i,j)]; }
    /// Returns the (i,j,k) element with () operator, i,j,k are zero based, row-major storage layout
    Scalar& operator()(int i, int j, int k) { return P_->buffer[P_->idx(i,j,k)]; }
    const Scalar& operator()(int i, int j, int k) const { return P_->buffer[P_->idx(i,j,k)]; }
    /// Returns a pointer to the data
    Scalar* data() { return P_->buffer.data(); }
    const Scalar* data() const { return P_->buffer.data(); }
    /// Adds the data from another array, sizes must match and the += operator must be applicable
    ArrayND& operator+=(const ArrayND& a) {
        if (size()==a.size()) {
            Scalar* p = data();
            Scalar* pend = p + size();
            const Scalar* q = a.data();
            while(p<pend) *p++ += *q++;
        }
        return *this;
    }
    /// Adds the data from another array, squared. Sizes must match and the += operator must be applicable
    void addSquared(const ArrayND& a) {
        if (size()==a.size()) {
            Scalar* p = data();
            Scalar* pend = p + size();
            const Scalar* q = a.data();
            while(p<pend) *p++ += (*q)*(*q++);
        }
    }
    /// Zero out the array
    void clear() {
        if (isNull()) return;
        Scalar* p = data();
        std::memset(p,0,size()*sizeof(Scalar));
    }

};

/// A typedef for N-d arrays of double numbers
typedef ArrayND<double> ArrayNDd;
/// A typedef for N-d arrays of float numbers
typedef ArrayND<float>  ArrayNDf;

#endif // ARRAYS_H
