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
    constexpr static int exponent_digits() { return (sizeof(float) << 3) - 1 - FLT_MANT_DIG; }
};
template <>
struct num_detail<double>
{
    constexpr static int mantissa_digits() { return DBL_MANT_DIG; }
    constexpr static int exponent_digits() { return (sizeof(double) << 3) - 1 - DBL_MANT_DIG; }
};
template <>
struct num_detail<long double>
{
    constexpr static int mantissa_digits() { return LDBL_MANT_DIG; }
    constexpr static int exponent_digits()
    {
        return (sizeof(long double) << 3) - 1 - LDBL_MANT_DIG;
    }
};

/**
 * @brief The corteo::iterator structure template implements \ref CorteoIdx in C++
 *
 * The corteo::iterator object can be used like a C++ iterator to loop over
 * the Corteo range range.
 *
 * At the same time, it provides access to the corresponding floating point value by means
 * of the dereference operator.
 *
 * To define an iterator object, we have to specify all template arguments, i.e.,
 * types of integral and real values, exponent min & max and bit number.
 * The following example creates two 4-bit Corteo iterators for 2^4+1=17 real values
 * ranging from 2^0 = 1 to 2^1 = 2.
 * Real and integral numeric types must be
 * of the same size. The
 * combinations `float, int` (32-bit) and `double, long` (64-bit) will generally work.
 *
 * \code
 *
 * #define N_BIT 4
 * #define EXP_MIN 0
 * #define EXP_MAX 1
 *
 * // iterator for float
 * corteo::iterator<float, int, N_BIT, EXP_MIN, EXP_MAX> if;
 *
 * // iterator for double
 * corteo::iterator<double, long, N_BIT, EXP_MIN, EXP_MAX> id;
 *
 * \endcode
 *
 * For example, a typical for-loop over the corteo range can be implemented
 * as follows:
 *
 * \code
 *
 * #define N_BIT 4
 * #define EXP_MIN 0
 * #define EXP_MAX 1
 *
 * corteo::iterator<float, int, N_BIT, EXP_MIN, EXP_MAX> i; // i initialized to 0
 *
 * for(; i<i.end(); i++) {
 *     float f = *i; // dereference operator returns the floating point number
 *     std::cout << f << std::endl;
 *     A[i] = ... // array indexing OK - implicit conversion to int
 * }
 *
 * \endcode
 *
 * The above code iterates over the defined corteo range and prints
 * all float values.
 *
 * The following can also be used:
 * \code
 * corteo::iterator<float, int, N_BIT, EXP_MIN, EXP_MAX> R;
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
struct iterator
{
    /// \brief Type of the floating-point numbers
    typedef _RealType RealType;
    /// \brief Type of the integral numbers
    typedef _IntType IntType;

    // compile-time checks
    static_assert(std::numeric_limits<RealType>::is_iec559,
                  "floating-point type is not IEC559/IEEE754 conformant");

    static_assert(sizeof(RealType) == sizeof(IntType),
                  "floating-point & integral types must be of the same size");

    static_assert(_maxExp > _minExp, "_maxExp must be larger than _minExp");

    static_assert(_maxExp <= std::numeric_limits<RealType>::max_exponent,
                  "_maxExp must less or equal to std::numeric_limits<RealType>::max_exponent");

    static_assert(_maxExp > 1 - std::numeric_limits<RealType>::max_exponent,
                  "_maxExp must grater than 1-std::numeric_limits<RealType>::max_exponent");

    static_assert(_minExp <= std::numeric_limits<RealType>::max_exponent,
                  "_minExp must less or equal to std::numeric_limits<RealType>::max_exponent");

    static_assert(_minExp > 1 - std::numeric_limits<RealType>::max_exponent,
                  "_minExp must grater than 1-std::numeric_limits<RealType>::max_exponent");

    // these constants are computed at compile time
    // this is ensured by the constexpr

    /// @brief Number of mantissa bits
    constexpr static const IntType Nbits = _Nb;
    /// @brief Number of mantissa values
    constexpr static const IntType Nm = (IntType(1) << _Nb);
    /// @brief Minimum real value in the range \f$ V_{min} = 2^{E_{min}} \f$
    constexpr static const RealType minVal = (_minExp >= IntType(0))
            ? IntType(1) << _minExp
            : RealType(1) / (IntType(1) << -_minExp);
    /// @brief Maximum real value in the range \f$ V_{max} = 2^{E_{max}} \f$
    constexpr static const RealType maxVal = (_maxExp >= IntType(0))
            ? IntType(1) << _maxExp
            : RealType(1) / (IntType(1) << -_maxExp);
    /// @brief Size of the range
    constexpr static const IntType size = (_maxExp - _minExp) * (IntType(1) << _Nb) + IntType(1);

private:
    // Exponent bias
    constexpr static const IntType bias =
            (IntType(std::numeric_limits<RealType>::max_exponent - 1) + _minExp) * Nm;
    // Exponent shift
    constexpr static const IntType shift =
            (IntType(num_detail<RealType>::mantissa_digits() - 1) - _Nb);
    //
    constexpr static const IntType dim = (_maxExp - _minExp) * Nm;

    constexpr static const IntType mantissa_mask = Nm - 1;
    constexpr static const IntType exp_mask =
            (IntType(1) << num_detail<RealType>::exponent_digits()) - 1;

    constexpr static auto log2_m_{ []() constexpr {
        std::array<RealType, Nm> w{};
        RealType f = RealType(1) / Nm;
        for (IntType i = 0; i < Nm; ++i)
            w[i] = std::log2(1 + f * i);
        return w;
    }() };

    /**
     * @brief Convert an index i to log2(xi)
     *
     * @param index
     * @return float
     */
    constexpr static RealType idx2log2(IntType index)
    {

        assert(index >= 0 && index <= dim);

        int m = index & mantissa_mask;
        int e = ((index >> Nbits) & exp_mask) + _minExp;

        return log2_m_[m] + e;
    }

    /**
     * @brief Convert float value to iterator
     *
     * The returned value is truncated to within the corteo range
     *
     * @param val a floating point value
     * @return unsigned int the corresponding index
     */
    constexpr static IntType val2idx(RealType val)
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
    constexpr static RealType idx2val(IntType index)
    {

        assert(index >= 0 && index <= dim);
        IntType ll = (index + bias) << shift;
        return *reinterpret_cast<RealType *>(&ll);
    }

public:
    /**
     * @brief Constructor with integral initializer.
     * @param i The initial value, defaults to 0
     */
    iterator(IntType i = IntType(0)) : i_(i) { }

    /**
     * @brief Constructor with floating-point initializer.
     *
     * The iterator's IntValue is initialized to fromValue() called with argument v.
     *
     * @param v The floating-point number
     */
    explicit iterator(const RealType &v) : i_(val2idx(v)) { }

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
    static iterator fromValue(RealType v) { return iterator(val2idx(v)); }

    /**
     * @brief Advance the iterator by one
     * @return A reference to the iterator
     */
    iterator &operator++()
    {
        i_++;
        return *this;
    }
    iterator operator++(int)
    {
        iterator retval = *this;
        ++(*this);
        return retval;
    }

    /**
     * @brief Reduce the iterator by one
     * @return A reference to the iterator
     */
    iterator &operator--()
    {
        i_--;
        return *this;
    }
    iterator operator--(int)
    {
        iterator retval = *this;
        --(*this);
        return retval;
    }

    /**
     * @brief Returns an iterator to the 1st point of the range
     */
    constexpr iterator begin() const { return iterator(IntType(0)); }

    /**
     * @brief Returns an iterator to one past the last point of the range
     */
    constexpr iterator end() const { return iterator(IntType(size)); }

    /**
     * @brief Returns an iterator to the last point of the range
     */
    constexpr iterator rbegin() const { return iterator(IntType(dim)); }

    /**
     * @brief Returns an iterator to one before the first point
     */
    constexpr iterator rend() const { return iterator(IntType(-1)); }

    /**
     * @brief The dereference operator * returns the corresponding real value
     */
    constexpr RealType operator*() { return idx2val(i_); }

    /**
     * @brief Implicit conversion to IntType
     */
    constexpr operator IntType() const { return i_; }

    /**
     * @brief Returns the corresponding real value
     */
    constexpr RealType value() const { return idx2val(i_); }

    /**
     * @brief Returns the corresponding real value
     */
    constexpr RealType log2v() const { return idx2log2(i_); }

    /**
     * @brief Returns the corresponding IntType
     */
    constexpr IntType toInt() const { return i_; }

private:
    IntType i_;
};

/**
 * @brief A linear interpolator for a corteo range
 *
 * @tparam iter_t the type of corteo::iterator
 *
 * @ingroup CorteoIdx
 */
template <class iter_t_>
class lin_interp
{

public:
    typedef iter_t_ iterator_type;
    typedef typename iter_t_::RealType RealType;

    /// @brief Default constructor, creates empty object
    lin_interp() = default;

    /**
     * @brief Construct a new lin_interp object to interpolate between the data given by y
     *
     * y is a numeric sequence which supports C-style (y[i]) 0-based indexing. The type of
     * the data elements must be convertible to RealType.
     *
     * The size of y must be equal or larger than that of \ref iterator_t.
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
        for (iterator_type i, j(1); i < i.end() - 1; i++, j++) {
            y_[i] = y[i];
            dydx_[i] = (y[j] - y[i]) / (*j - *i);
        }
        y_[iterator_type::size - 1] = y[iterator_type::size - 1];
    }

    /// @brief Returns the linearly interpolated value \f$ y = y(x) \f$
    RealType operator()(const RealType &x) const
    {
        if (x <= iterator_type::minVal)
            return y_.front();
        if (x >= iterator_type::maxVal)
            return y_.back();
        iterator_type i(x);
        return y_[i] + dydx_[i] * (x - *i);
    }

    /// @brief Returns a pointer to the internal data table
    const RealType *data() const { return &y_[0]; }

private:
    std::array<RealType, iterator_type::size> y_, dydx_;
};

/**
 * @brief A log-log interpolator for a corteo range
 *
 * @tparam idx_t is the type of corteo::iterator
 *
 * @ingroup CorteoIdx
 */
template <class iter_t_>
class log_interp
{

public:
    typedef iter_t_ iterator_type;
    typedef typename iter_t_::RealType RealType;

    /// @brief Default constructor, creates empty object
    log_interp() = default;

    /**
     * @brief Construct a new log_interp object to perform log-log interpolation with the data given
     * by y
     *
     * y is a numeric sequence which supports C-style (y[i]) 0-based indexing. The type of
     * the data elements must be convertible to float. log(y[i]) must be finite for all i in the
     * range of \ref iterator_type.
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
        for (iterator_type i, j(1); i < i.end() - 1; i++, j++) {
            d_[i] = (std::log2(y[j]) - std::log2(y[i])) / (std::log2(*j) - std::log2(*i));
            y_[i] = y[i];
        }
        y_[iterator_type::size - 1] = y[iterator_type::size - 1];
    }

    /// @brief Returns the log-log interpolated value y(x)
    RealType operator()(const RealType &x) const
    {
        if (x <= iterator_type::minVal)
            return y_.front();
        if (x >= iterator_type::maxVal)
            return y_.back();
        iterator_type i(x);
        return y_[i] * std::exp2(d_[i] * std::log2(x / (*i)));
    }

    /// @brief Returns a pointer to the internal data table
    const RealType *data() const { return &y_[0]; }

private:
    std::array<RealType, iterator_type::size> y_, d_;
};

} // namespace corteo

#endif
