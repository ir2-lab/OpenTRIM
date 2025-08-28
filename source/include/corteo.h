#ifndef _CORTEO_H_
#define _CORTEO_H_

#include <cassert>
#include <limits>
#include <cfloat>
#include <array>
#include <cmath>

namespace corteo {

/**
 * \defgroup CorteoIdx Corteo indexing
 *
 * \brief A fast indexing method for log-spaced ranges.
 * @{
 *
 * It was originally proposed by Yuan et al NIMB83(1993) p.413 for indexing
 * of tabulated
 * scattering cross sections over wide ranges of energy and impact parameter.
 *
 * The implentation here is based on the program Corteo, written by Francois Schiettekatte
 * and released under the GNU GPL.
 * The original corteo source code can be found here:
 * http://www.lps.umontreal.ca/~schiette/index.php?n=Recherche.Corteo
 *
 * The indexing method utilizes the binary floating-point number representation
 * used in most computer systems which is based on the IEC559/IEEE754 standard.
 *
 * The IEEE754 float has the form
 * \f[
 * (-1)^s \times m \times 2^E
 * \f]
 * where \f$ s \f$ is the sign bit, \f$ m \f$ is the "mantissa" and \f$ E \f$ the exponent in the
 * range \f$ [1-E_{max}, E_{max}] \f$.
 *
 * For the subset of "normal" floats
 * \f[
 * m = 1 + k/2^n : 0 \leq k < 2^n
 * \f]
 * where \f$ n \f$ is the number of mantissa bits.
 *
 * The 32-bit IEEE754 float has \f$ E_{max}=\f$
 * FLT_MAX_EXP - 1 = 127  and \f$ n=\f$
 * FLT_MANT_DIG - 1 = 23.
 *
 * The Corteo indexing specifies a correspondence between an integer sequence
 * \f$ I = 0, 1, 2, \dots D \f$
 * and a range of real numbers \f$ X_I \f$, \f$ 2^{E_{min}} \leq X_I \leq 2^{E_{max}} \f$,
 * where
 * \f{eqnarray*}{
 *   X_I &=& m \times 2^E \\
 *   m &=& 1 + (I \bmod 2^N) \times 2^{-N}  \\
 *   E &=& E_{min} + (I \div 2^N) \\
 *   D &=& 2^N\left(E_{max} - E_{min}\right)
 * \f}
 *
 * The form of \f$ X_I \f$ is very similar to the IEEE754 representation with \f$ N \f$ mantissa
 * bits. Thus, \f$ I \f$ can be converted to \f$ X_I \f$ and vice-versa by fast bit manipulation
 * operations.
 *
 * An example with \f$ N=2 \f$, \f$ E_{min}=0 \f$, \f$ E_{max}=2 \f$ is shown in the table:
 *
 *
 * | 2-bit corteo range from 2^0 to 2^4 ||||||||||
 * | :- | :-: | :-:  | :-:  | :-:  | :-:  | :-:  | :-:  | :-:  | :-: |
 * | \f$ I \f$  | 0   | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8   |
 * | \f$ X_I \f$ | 1   | 1.25 | 1.5 | 1.75 | 2 | 2.5 | 3 | 3.5 | 4 |
 *
 *
 * Thus, it is evident that this type of indexing can be used for fast lookup of logarithmic tables,
 * e.g., in interpolation schemes, without the need to call the log() function.
 *
 * Tabulating a scattering cross-section over a wide range of particle energies spanning several
 * orders of magnitude is such an application.
 *
 * @}
 *
 */

// get templated function for the # of mantissa digits
template <class T>
struct num_detail;
template <>
struct num_detail<float>
{
    constexpr static int mantissa_digits() { return FLT_MANT_DIG; }
};
template <>
struct num_detail<double>
{
    constexpr static int mantissa_digits() { return DBL_MANT_DIG; }
};
template <>
struct num_detail<long double>
{
    constexpr static int mantissa_digits() { return LDBL_MANT_DIG; }
};

/**
 * @brief The corteo::index structure template implements \ref CorteoIdx in C++
 *
 * The corteo::index object can be used like a C++ iterator to loop over
 * a log-spaced range.
 *
 * At the same time, it provides access to the corresponding floating point value by means
 * of the dereference operator.
 *
 * For example, a typical for-loop over the corteo range can be implemented
 * as follows:
 *
 * \code
 * corteo::index<float, int, 4, 0, 1> i; // i initialized to 0
 * for(; i<i.end(); i++) {
 *     float f = *i; // dereference operator returns the floating point number
 *     std::cout << f << std::endl;
 *     A[i] = ... // array indexing OK - implicit conversion to int
 * }
 * \endcode
 *
 * The above code iterates over the defined corteo range and prints
 * all 2^4+1=17 float values from 2^0 = 1 to 2^1 = 2.
 *
 * The following can also be used:
 * \code
 * corteo_index<float, int, 4, 0, 2> R;
 * for(const float& f : R) {
 *     std::cout << f << std::endl;
 * }
 * \endcode
 *
 * The value of std::numeric_limits::is_iec559
 * defined in the <limits> standard library header
 * is checked at compile-time for compatibility
 * of the underlying implementation with the IEEE-754 std.
 *
 * The template parameters RealType and IntType must
 * specify real and integral numeric types, respectively,
 * of the same size. The
 * combinations float-int and double-long will generally work.
 *
 * @todo
 * Include a compile-time C++ check for endianess.
 * Currently gcc in Ubuntu 22.04 does not have std::endian
 *
 *
 * @tparam RealType floating-point type
 * @tparam IntType signed integral type for the index
 * @tparam _Nb # of mantissa bits
 * @tparam _minExp Exponent of the min value = 2^minExp
 * @tparam _maxExp Exponent of the max value = 2^maxExp
 *
 * @ingroup CorteoIdx
 */
template <class _RealType, class _IntType, _IntType _Nb, _IntType _minExp, _IntType _maxExp>
struct index
{
    typedef _RealType RealType;
    typedef _IntType IntType;

    // compile-time checks
    static_assert(std::numeric_limits<RealType>::is_iec559,
                  "floating-point type is not IEC559/IEEE754 conformant");
    static_assert(_maxExp > _minExp, "_maxExp must be larger than _minExp");
    static_assert(sizeof(RealType) == sizeof(IntType), "real & int types must have the same size");

    // these constants are computed at compile time
    // this is ensured by the constexpr

    /// @brief Number of mantissa bits
    constexpr static const IntType Nbits = _Nb;
    /// @brief Minimum real value in the range =  2^minExp
    constexpr static const RealType minVal = (_minExp >= IntType(0))
            ? IntType(1) << _minExp
            : RealType(1) / (IntType(1) << -_minExp);
    /// @brief Maximum real value in the range = 2^maxExp
    constexpr static const RealType maxVal = (_maxExp >= IntType(0))
            ? IntType(1) << _maxExp
            : RealType(1) / (IntType(1) << -_maxExp);
    /// @brief Size of indexing range
    constexpr static const IntType size = (_maxExp - _minExp) * (IntType(1) << _Nb) + IntType(1);

private:
    /// @brief Exponent bias
    constexpr static const IntType bias =
            (IntType(std::numeric_limits<RealType>::max_exponent - 1) + _minExp)
            * (IntType(1) << _Nb);
    /// @brief Exponent shift
    constexpr static const IntType shift =
            (IntType(num_detail<RealType>::mantissa_digits() - 1) - _Nb);
    /// @brief Size of indexing range
    constexpr static const IntType dim = (_maxExp - _minExp) * (IntType(1) << _Nb);

    /**
     * @brief Convert float value to index
     *
     * The returned value is truncated to within the corteo range
     *
     * @param val a floating point value
     * @return unsigned int the corresponding index
     */
    static IntType val2idx(RealType val)
    {
        if (val <= minVal)
            return IntType(0);
        if (val >= maxVal)
            return dim;
        IntType ll = *reinterpret_cast<IntType *>(&val);
        ll = (ll >> shift) - bias;
        return ll;
    }

    /**
     * @brief Convert an index to a floating point value
     *
     * @param index
     * @return float
     */
    static RealType idx2val(IntType index)
    {

        assert(index >= 0 && index <= dim);
        IntType ll = (index + bias) << shift;
        return *reinterpret_cast<RealType *>(&ll);
    }

public:
    /**
     * @brief Constructor with index initializer.
     * @param i The initial value of the index, defaults to 0
     */
    index(IntType i = IntType(0)) : i_(i) { }

    /**
     * @brief Constructs an index that corresponds to float v
     *
     * The index corresponds to fromValue() called with argument v
     *
     * @param v The floating point number
     */
    explicit index(const RealType &v) : i_(val2idx(v)) { }

    /**
     * @brief Helper function to convert a real number to index
     *
     * The function finds the real number X_I within the corteo range
     * which is closest to v and returns the corresponding index I.
     *
     * See \ref CorteoIdx for more details.
     *
     * If v is smaller than corteo_index::minVal the function returns begin().
     *
     * If v is larger than corteo_index::maxVal the function returns end().
     *
     * @param v is the floating-point number
     * @return the corresponding index
     */
    static index fromValue(RealType v) { return index(val2idx(v)); }

    /**
     * @brief Advance the index by one
     * @return A reference to the index
     */
    index &operator++()
    {
        i_++;
        return *this;
    }
    index operator++(int)
    {
        index retval = *this;
        ++(*this);
        return retval;
    }

    /**
     * @brief Reduce the index by one
     * @return A reference to the index
     */
    index &operator--()
    {
        i_--;
        return *this;
    }
    index operator--(int)
    {
        index retval = *this;
        --(*this);
        return retval;
    }

    /**
     * @brief Returns an index to the start of the range, i.e. 0
     * @return A corteo_index pointing to 0
     */
    constexpr index begin() const { return index(IntType(0)); }

    /**
     * @brief Returns an index to one past the last point of the range, i.e. corteo_index::size
     * @return A corteo_index pointing to one past the end of the range
     */
    constexpr index end() const { return index(IntType(size)); }

    /**
     * @brief Returns an index to the last point of the range, i.e. corteo_index::size-1
     * @return A corteo_index pointing to the last point
     */
    constexpr index rbegin() const { return index(IntType(dim)); }

    /**
     * @brief Returns an index to the position before the first point, i.e. -1
     */
    constexpr index rend() const { return index(IntType(-1)); }

    /**
     * @brief The dereference operator * returns the corresponding real value
     * @return The real value corresponding to the corteo index
     */
    RealType operator*() { return idx2val(i_); }

    /**
     * @brief Implicit conversion to integral type IntType (typically int)
     */
    operator IntType() const { return i_; }

    /**
     * @brief Returns the corresponding real value
     * @return The real value corresponding to the corteo index
     */
    RealType value() const { return idx2val(i_); }

    /**
     * @brief Returns the index as an IntType
     */
    IntType toInt() const { return i_; }

private:
    IntType i_;
};

/**
 * @brief A linear interpolator for a corteo range
 *
 * @tparam idx_t the type of corteo::index
 *
 * @ingroup CorteoIdx
 */
template <class idx_t>
class lin_interp
{

public:
    typedef typename idx_t::RealType RealType;

    /// @brief Default constructor, creates empty object
    lin_interp() = default;

    /**
     * @brief Construct a new lin_interp object to interpolate between the data given by y
     *
     * y is a numeric sequence which supports C-style (y[i]) 0-based indexing. The type of
     * the data elements must be convertible to float.
     *
     * The size of y must be equal or larger than that of the corteo index \ref idx_t.
     *
     * y is copied to an internal buffer.
     *
     * @tparam sequence_t a container type with C-style indexing
     * @param y the data to interpolate
     */
    template <class sequence_t>
    explicit lin_interp(const sequence_t &y)
    {
        set(y);
    }

    /**
     * @brief Call this function to set new interpolator data
     *
     * y is a numeric sequence which supports C-style (y[i]) 0-based indexing. The type of
     * the data elements must be convertible to float.
     *
     * The size of y must be equal or larger than that of the corteo index \ref idx_t.
     *
     * y is copied to an internal buffer.
     *
     * @tparam sequence_t a container type with C-style indexing
     * @param y the data to interpolate
     */
    template <class sequence_t>
    void set(const sequence_t &y)
    {
        for (idx_t i, j(1); i < i.end() - 1; i++, j++) {
            y_[i] = y[i];
            dydx_[i] = (y[j] - y[i]) / (*j - *i);
        }
        y_[idx_t::size - 1] = y[idx_t::size - 1];
    }

    /// @brief Returns the linearly interpolated value \f$ y = y(x) \f$
    RealType operator()(const RealType &x) const
    {
        if (x <= idx_t::minVal)
            return y_.front();
        if (x >= idx_t::maxVal)
            return y_.back();
        idx_t i(x);
        return y_[i] + dydx_[i] * (x - *i);
    }

    /// @brief Returns a pointer to the internal data table
    const RealType *data() const { return &y_[0]; }

private:
    std::array<RealType, idx_t::size> y_, dydx_;
};

/**
 * @brief A log-log interpolator for a corteo range
 *
 * @tparam idx_t is the type of corteo::index
 *
 * @ingroup CorteoIdx
 */
template <class idx_t>
class log_interp
{

public:
    typedef typename idx_t::RealType RealType;

    /// @brief Default constructor, creates empty object
    log_interp() = default;

    /**
     * @brief Construct a new log_interp object to perform log-log interpolation with the data given
     * by y
     *
     * y is a numeric sequence which supports C-style (y[i]) 0-based indexing. The type of
     * the data elements must be convertible to float. log(y[i]) must be finite for all i in the
     * range of \ref idx_t.
     *
     * The size of y must be equal or larger than that of the corteo index \ref idx_t.
     * The fisrt corteo_index::size elements of y will be used.
     *
     * y is copied to an internal buffer.
     *
     * @tparam sequence_t a container type with C-style indexing
     * @param y the data to interpolate
     */
    template <class sequence_t>
    explicit log_interp(const sequence_t &y)
    {
        set(y);
    }

    /**
     * @brief Call this function to set new interpolator data
     *
     * y is a numeric sequence which supports C-style (y[i]) 0-based indexing. The type of
     * the data elements must be convertible to float.
     *
     * log(y[i]) must be finite for all i in the range
     *
     * The size of y must be equal or larger than that of the corteo index \ref idx_t.
     * The fisrt corteo_index::size elements of y will be used.
     *
     * y is copied to an internal buffer.
     *
     * @tparam sequence_t a container type with C-style indexing
     * @param y the data to interpolate
     */
    template <class sequence_t>
    void set(const sequence_t &y)
    {
        for (idx_t i, j(1); i < i.end() - 1; i++, j++) {
            d_[i] = (std::log2(y[j]) - std::log2(y[i])) / (std::log2(*j) - std::log2(*i));
            y_[i] = y[i];
        }
        y_[idx_t::size - 1] = y[idx_t::size - 1];
    }

    /// @brief Returns the log-log interpolated value y(x)
    RealType operator()(const RealType &x) const
    {
        if (x <= idx_t::minVal)
            return y_.front();
        if (x >= idx_t::maxVal)
            return y_.back();
        idx_t i(x);
        return y_[i] * std::exp2(d_[i] * std::log2(x / (*i)));
    }

    /// @brief Returns a pointer to the internal data table
    const RealType *data() const { return &y_[0]; }

private:
    std::array<RealType, idx_t::size> y_, d_;
};

} // namespace corteo

#endif
