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
 * that may be shared read-only among threads of execution.
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
template <typename Scalar>
class ArrayND
{
    typedef ArrayND<Scalar> _Self;

    enum StorageOrder { RowMajor, ColumnMajor };

    struct array_nd
    {
        StorageOrder storage_order{ RowMajor };
        std::vector<size_t> dim;
        std::vector<size_t> stride;
        std::vector<Scalar> buffer;

        array_nd() = default;
        array_nd(const array_nd &other) = default;

        array_nd(size_t i, StorageOrder O)
            : storage_order(O), dim({ i }), stride({ 1 }), buffer(i, Scalar(0))
        {
        }

        array_nd(size_t i, size_t j, StorageOrder O)
            : storage_order(O),
              dim({ i, j }),
              stride(calc_stride(dim, storage_order)),
              buffer(i * j, Scalar(0))
        {
        }

        array_nd(size_t i, size_t j, size_t k, StorageOrder O)
            : storage_order(O),
              dim({ i, j, k }),
              stride(calc_stride(dim, storage_order)),
              buffer(i * j * k, Scalar(0))
        {
        }

        array_nd(const std::vector<size_t> &d, StorageOrder O)
            : storage_order(O),
              dim(d),
              stride(calc_stride(dim, storage_order)),
              buffer(calc_size(d), Scalar(0))
        {
        }

        void resize(const std::vector<size_t> &d)
        {
            dim = d;
            stride = calc_stride(d, storage_order);
            buffer.resize(calc_size(d));
        }

        bool reshape(const std::vector<size_t> &d)
        {
            if (calc_size(d) != buffer.size())
                return false;
            dim = d;
            stride = calc_stride(d, storage_order);
            return true;
        }

        size_t idx(int i, int j)
        {
            assert(dim.size() == 2);
            return i * stride[0] + j * stride[1];
        }
        size_t idx(int i, int j, int k)
        {
            assert(dim.size() == 3);
            return i * stride[0] + j * stride[1] + k * stride[2];
        }
        int idx(const std::vector<int> &d) const
        {
            assert(dim.size() == d.size());
            int k = 0;
            for (int i = 0; i < d.size(); i++)
                k += stride[i] * d[i];
            return k;
        }
        size_t idx(const std::vector<size_t> &d) const
        {
            assert(dim.size() == d.size());
            size_t k = 0;
            for (int i = 0; i < d.size(); i++)
                k += stride[i] * d[i];
            return k;
        }
        void copyTo(array_nd &other)
        {
            std::memcpy(other.buffer.data(), buffer.data(), buffer.size() * sizeof(Scalar));
        }
        static size_t calc_size(const std::vector<size_t> &d)
        {
            size_t S(1);
            for (auto s : d)
                S *= s;
            return S;
        }
        static std::vector<size_t> calc_stride(const std::vector<size_t> &d, StorageOrder ord)
        {
            size_t n = d.size();
            std::vector<size_t> s(n);
            if (ord == RowMajor) {
                auto si = s.rbegin();
                auto di = d.rbegin();
                *si = 1;
                si++;
                while (si != s.rend()) {
                    *si = (*(si - 1)) * (*di);
                    si++;
                    di++;
                }
            } else { // ColumnMajor
                auto si = s.begin();
                auto di = d.begin();
                *si = 1;
                si++;
                while (si != s.end()) {
                    *si = (*(si - 1)) * (*di);
                    si++;
                    di++;
                }
            }
            return s;
        }
    };

    std::shared_ptr<array_nd> P_;

    explicit ArrayND(const array_nd &p) : P_(new array_nd(p)) { }

public:
    /// Constructs an empty array
    ArrayND() = default;

    /// Constructs an N-dimensional array of dimensions defined by dim
    explicit ArrayND(const std::vector<size_t> &dim, StorageOrder O = RowMajor)
        : P_(new array_nd(dim, O))
    {
    }
    /// Constructs a 1D array of dimension i
    explicit ArrayND(int i, StorageOrder O = RowMajor) : P_(new array_nd(i, O)) { }
    /// Constructs a 2D array of dimensions i x j
    explicit ArrayND(int i, int j, StorageOrder O = RowMajor) : P_(new array_nd(i, j, O)) { }
    /// Constructs a 3D array of dimensions i x j x k
    explicit ArrayND(int i, int j, int k, StorageOrder O = RowMajor) : P_(new array_nd(i, j, k, O))
    {
    }
    /// Copy constructor. Data is shared.
    ArrayND(const ArrayND &other) : P_(other.P_) { }
    /// Assignement operator. Data is shared.
    ArrayND &operator=(const ArrayND &other)
    {
        P_ = other.P_;
        return *this;
    }
    /// Creates a copy of the array and returns it
    ArrayND copy() const { return P_ ? ArrayND(*P_) : ArrayND(); }
    /// Copies the array into \a other. If \a other does not have
    /// the neccesary size, the function does nothing & returns false.
    bool copyTo(ArrayND &other) const
    {
        bool ret = P_ && other.P_ && size() == other.size();
        if (ret)
            P_->copyTo(*(other.P_));
        return ret;
    }
    /// Returns true is the array is empty
    bool isNull() const { return P_ == nullptr; }
    /// Returns the dimensions of the array
    const std::vector<size_t> &dim() const { return P_->dim; }
    /// Returns the stride vector of the array
    const std::vector<size_t> &stride() const { return P_->stride; }
    /// Returns the total number of elements
    size_t size() const { return P_->buffer.size(); }
    /// Returns the number of dimensions
    int ndim() const { return P_->dim.size(); }
    /// Returns the storage order
    StorageOrder storage_order() const { return P_->storage_order; }

    /// Reshape the array
    bool reshape(const std::vector<size_t> &d) { return P_->reshape(d); }

    /// Returns the i-th element in C-style operator
    Scalar &operator[](int i) { return P_->buffer[i]; }
    const Scalar &operator[](int i) const { return P_->buffer[i]; }
    /// Returns the i-th element with () operator, i is zero based
    Scalar &operator()(int i) { return P_->buffer[i]; }
    const Scalar &operator()(int i) const { return P_->buffer[i]; }
    /// Returns the (i,j) element with () operator, i,j are zero based
    Scalar &operator()(int i, int j) { return P_->buffer[P_->idx(i, j)]; }
    const Scalar &operator()(int i, int j) const { return P_->buffer[P_->idx(i, j)]; }
    /// Returns the (i,j,k) element with () operator, i,j,k are zero based
    Scalar &operator()(int i, int j, int k) { return P_->buffer[P_->idx(i, j, k)]; }
    const Scalar &operator()(int i, int j, int k) const { return P_->buffer[P_->idx(i, j, k)]; }
    /// Returns the element at multidimensional index i with () operator, indices are zero based
    Scalar &operator()(const std::vector<size_t> i) { return P_->buffer[P_->idx(i)]; }
    const Scalar &operator()(const std::vector<size_t> i) const { return P_->buffer[P_->idx(i)]; }

    /// Returns a pointer to the data
    Scalar *data() { return P_->buffer.data(); }
    const Scalar *data() const { return P_->buffer.data(); }
    /// Adds the data from another array, sizes must match and the += operator must be applicable
    ArrayND &operator+=(const ArrayND &a)
    {
        if (size() == a.size()) {
            Scalar *p = data();
            Scalar *pend = p + size();
            const Scalar *q = a.data();
            while (p < pend)
                *p++ += *q++;
        }
        return *this;
    }
    /// Adds the data from another array, squared. Sizes must match and the += operator must be
    /// applicable
    void addSquared(const ArrayND &a)
    {
        if (size() == a.size()) {
            Scalar *p = data();
            Scalar *pend = p + size();
            const Scalar *q = a.data();
            while (p < pend) {
                *p++ += (*q) * (*q);
                q++;
            }
        }
    }
    /// Zero out the array
    void clear()
    {
        if (isNull())
            return;
        Scalar *p = data();
        std::memset(p, 0, size() * sizeof(Scalar));
    }

    long use_count() const { return P_.use_count(); }
};

/// A typedef for N-d arrays of double numbers
typedef ArrayND<double> ArrayNDd;
/// A typedef for N-d arrays of float numbers
typedef ArrayND<float> ArrayNDf;

#endif // ARRAYS_H
