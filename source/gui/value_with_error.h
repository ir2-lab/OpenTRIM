#ifndef VALUE_WITH_ERROR_H
#define VALUE_WITH_ERROR_H

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <string>
#include <sstream>

/**
 * @brief A real value with error bound
 *
 * It can be used with real numeric types: float, double, long double
 *
 * The real value is rounded to the error precision.
 *
 * The value with the error can be printed to a text stream with various formatting options.
 *
 * @tparam T the floating point type
 */
template <class T>
struct value_with_error : std::pair<T, T>
{
    /*
     * Return x and exp so that
     *      * num = x * 10^(*exp)
     *      * with 0.1 <= |x| < 1
     *      */
    constexpr static T frexp10(const T &num, int *exp)
    {
        if (num == T(0)) { // similar to frexp
            *exp = 0;
            return T(0);
        }
        T logn = std::log10(std::abs(num));
        *exp = std::floor(logn) + 1;
        return num * std::pow(T(10), -(*exp));
    }

    static void print_superscript(std::ostream &os, int i)
    {
        static const char *ss[] = { "\u2070", "\u00b9", "\u00b2", "\u00b3", "\u2074", "\u2075",
                                    "\u2076", "\u2077", "\u2078", "\u2079", "\u207a", "\u207b" };

        static constexpr int buff_len = 32;
        char buff[buff_len]{};
        sprintf(buff, "%d", std::abs(i));
        if (i < 0)
            os << ss[11];
        for (char *p = buff; *p != 0; ++p)
            os << ss[*p - 48];
    }

    void print_to_fixed(std::ostream &os) const
    {
        std::ios_base::fmtflags oldfmt = os.flags();
        int oldp = os.precision();

        os << std::fixed;
        if (fmt_ == parenthesis) {
            os << std::setprecision(value_precision) << this->first;
            os << '(';
            os << std::setprecision(err_precision) << this->second;
            os << ')';
        } else {
            os << std::setprecision(value_precision) << this->first;
            os << "±";
            os << std::setprecision(err_precision) << this->second;
        }

        os.setf(oldfmt, std::ios_base::floatfield);
        os << std::setprecision(oldp);
    }

public:
    enum format { parenthesis, plusminus };

    typedef std::ios_base &(*iosfmt)(std::ios_base &);

    /**
     * @brief Round a value according to its error
     *
     * The error df is rounded to d significant digits.
     * Default d=2 but d=1 is also often used.
     *
     * f is rounded to the same order as df.
     *
     * @param f real value
     * @param df error of f
     * @param d error value significant digits
     * @return T the rounded value of f
     */
    static T round_with_err(const T &f, const T &df, int d = 2)
    {
        if (df <= 0)
            return f;

        // split numbers to mantissa and exponent
        int n, dn;
        T x = frexp10(f, &n);
        T dx = frexp10(df, &dn);

        // find the required precision = # of
        // significant digits to express
        // the value within error.
        // min precision is 1 (1 digit for the result)
        // round f to the precision
        int precision = std::max(n - dn + d, 1);
        T sc = std::pow(T(10), precision - n);
        return std::round(f * sc) / sc;
    }

    /**
     * @brief Construct a new value_with_error object
     *
     * @a iosf and @a with_parenthesis define the output format.
     *
     * Example:
     * The value 0.0123 with error 0.0045 will be printed as
     *   - 0.012(5) with iosf=fixed or defaultfloat, error_digits=1, parenthesis=true
     *   - 0.012±0.005 with iosf=fixed or defaultfloat, error_digits=1, parenthesis=false
     *   - 1.23(45)×10⁻² with iosf=scientific, error_digits=2, parenthesis=true
     *   - (1.23±0.45)×10⁻² with fmt=scientific, error_digits=2, parenthesis=false
     *
     * @param f a real value
     * @param df error of f
     * @param error_digits sighificant digits of df
     * @param iosf ios float format (fixed, scientific or default)
     * @param par if true the value will be printed with parentheses, otherwise with a ± sign
     */
    value_with_error(const T &f, const T &df, int error_digits = 2, iosfmt iosf = std::defaultfloat,
                     bool with_parenthesis = false)
        : std::pair<T, T>(f, df),
          iosfmt_(iosf),
          err_digits_(error_digits),
          fmt_(with_parenthesis ? parenthesis : plusminus)
    {
        // split numbers to mantissa and exponent
        int n, dn;
        T x = frexp10(f, &n);
        T dx = frexp10(df, &dn);

        // round error to the required precision
        T sc = std::pow(T(10), error_digits);
        dx = std::round(dx * sc) / sc;

        // find the required precision = # of
        // significant digits to express
        // the value + error.
        // min precision is 1 (1 digit for the result)
        // round f to the precision
        int precision = std::max(n - dn + error_digits, 1);
        sc = std::pow(T(10), precision);
        x = std::round(x * sc) / sc;

        if (iosf == std::defaultfloat) {
            iosfmt_ = ((precision > n - 1) && (n >= -3)) ? std::fixed : std::scientific;
        }

        if (fmt_ == parenthesis) {
            if (iosfmt_ == std::fixed) {
                // convert x to the rounded f value
                x *= std::pow(T(10), n);
                dx *= std::pow(T(10), dn);

                // find required # of decimals
                int decimals = std::max(precision - n, 0);

                if (decimals) {
                    if (error_digits <= decimals) {
                        dx *= std::pow(T(10), decimals);
                        value_precision = decimals;
                        err_precision = 0;
                    } else {
                        value_precision = decimals;
                        err_precision = decimals;
                    }
                } else {
                    value_precision = err_precision = 0;
                }

            } else if (iosfmt_ == std::scientific) {
                x *= std::pow(T(10), 1);
                // dx *= std::pow(T(10),dn+1-n);

                int decimals = std::max(precision - 1, 0);

                if (decimals) {
                    if (error_digits <= decimals) {
                        dx *= std::pow(T(10), error_digits);
                        value_precision = decimals;
                        err_precision = 0;
                    } else {
                        dx *= std::pow(T(10), error_digits - decimals);
                        value_precision = decimals;
                        err_precision = decimals;
                    }
                } else {
                    dx *= std::pow(T(10), dn + 1 - n);
                    value_precision = err_precision = 0;
                }
            }

        } else { // plusminus

            if (iosfmt_ == std::fixed) {
                // convert x,dx to the rounded f,df values
                x *= std::pow(T(10), n);
                dx *= std::pow(T(10), dn);

                // find required # of decimals
                int decimals = std::max(precision - n, 0);

                if (decimals) {
                    value_precision = decimals;
                    err_precision = decimals;
                } else {
                    value_precision = err_precision = 0;
                }

            } else if (iosfmt_ == std::scientific) {
                x *= std::pow(T(10), 1);
                dx *= std::pow(T(10), dn + 1 - n);

                int decimals = std::max(precision - 1, 0);

                if (decimals) {
                    value_precision = decimals;
                    err_precision = decimals;
                } else {
                    value_precision = err_precision = 0;
                }
            }
        }

        this->first = x;
        this->second = dx;
        exp_ = n - 1;
    }

    void print_to(std::ostream &os) const
    {
        if (fmt_ == parenthesis) {
            print_to_fixed(os);
            if (iosfmt_ == std::scientific && exp_ != 0) {
                os << "×10";
                print_superscript(os, exp_);
            }
        } else {
            if (iosfmt_ == std::fixed)
                print_to_fixed(os);
            else {
                if (exp_ != 0) {
                    os << '(';
                    print_to_fixed(os);
                    os << ")×10";
                    print_superscript(os, exp_);
                } else
                    print_to_fixed(os);
            }
        }
    }

    std::string to_string() const
    {
        std::ostringstream ss;
        print_to(ss);
        return ss.str();
    }

private:
    iosfmt iosfmt_;
    int err_digits_;
    format fmt_;
    int value_precision, err_precision;
    int exp_;
};

template <class T>
std::ostream &operator<<(std::ostream &os, const value_with_error<T> &v)
{
    v.print_to(os);
    return os;
}

#endif // VALUE_WITH_ERROR_H
