#ifndef ERROR_FMT_H
#define ERROR_FMT_H

#include <cmath>
#include <cstring>
#include <string>
#include <charconv>

struct error_fmt
{
    /*
     * Return x and exp so that
     *      * num = x * 10^(*exp)
     *      * with 0.1 <= |x| < 1
     *      */
    template <class T>
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

    /*
     * Round a value according to its error
     *      * f is a value with error df, round df to d significant
     * digits and then round f to the same order as
     * the error df.
     *      * Default d=2. d=1 is also often used.
     *      */
    template <class T>
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

    static inline std::string superscript(int i)
    {
        static const char *ss[] = { "\u2070", "\u00b9", "\u00b2", "\u00b3", "\u2074", "\u2075",
                                    "\u2076", "\u2077", "\u2078", "\u2079", "\u207a", "\u207b" };
        static constexpr int buff_len = 32;
        char buff[buff_len]{};
        std::to_chars_result ret = std::to_chars(buff, buff + buff_len, std::abs(i));
        *ret.ptr = 0;
        char *p = buff;
        std::string s;
        if (i < 0)
            s += ss[11];
        while (*p) {
            int k = *p - 48;
            s.append(std::string(ss[k]));
            p++;
        }
        return s;
    }

    /*
     *  Print real value with error - base implementation
     *
     *  Based on C++17 std::to_chars to convert numbers to string
     *  This avoids interference with system locale.
     *  Numbers are always in "C" locale, e.g., 1000.345
     *
     */
    template <class T>
    static std::string print_with_err_impl(const T &f1, int d1, const T &f2, int d2,
                                           std::chars_format fmt, bool parenthesis)
    {
        constexpr size_t buff_size = 1024;
        char buff[buff_size]{};
        char *buff_end = buff + buff_size;
        char *pbuff = buff;

        std::to_chars_result result;

        if (parenthesis) {
            result = std::to_chars(pbuff, buff_end, f1, fmt, d1);
            pbuff = result.ptr;
            *pbuff++ = '(';
            result = std::to_chars(pbuff, buff_end, f2, fmt, d2);
            pbuff = result.ptr;
            *pbuff++ = ')';
            *pbuff = 0;
        } else {
            constexpr const char *pm = "±";
            size_t pmlen = std::strlen(pm);

            result = std::to_chars(pbuff, buff_end, f1, fmt, d1);
            pbuff = result.ptr;
            std::memcpy(pbuff, pm, pmlen);
            pbuff += pmlen;
            result = std::to_chars(pbuff, buff_end, f2, fmt, d2);
            pbuff = result.ptr;
            *pbuff = 0;
        }

        return std::string(buff);
    }

    /**
     * @brief Print real value with error
     *
     *      * Works for real numric types T = float, double, long double
     *      * Prints the real value f and error df and returns as a std::string.
     *      * The @a fmt option specifies fixed, scientific or general notation
     *      * The value of @a error_digits corresponds to the significant digits in the error @a df.
     * Typically @a error_digits is equal to 2 (default) or 1.
     *      * The @a parenthesis option selects between two modes of error reporting.
     *      * Example:
     *      * The value 0.0123 with error 0.0045 will be printed as
     *   - 0.012(5) with fmt=fixed or general, error_digits=1, parenthesis=true
     *   - 0.012±0.005 with fmt=fixed or general, error_digits=1, parenthesis=false
     *   - 1.23(45)*1e-2 with fmt=scientific, error_digits=2, parenthesis=true
     *   - (1.23±0.45)*1e-2 with fmt=scientific, error_digits=2, parenthesis=false
     *      * @param f value to be printed
     * @param df error of f, also printed
     * @param fmt c++ std numeric format specifier (std::char_format)
     * @param error_digits # of significant digits in df
     * @param parenthesis use parenthesis style when printing the error, otherwise use ±
     * @return a std::string containing the printed value
     */
    template <class T>
    static std::string print_with_err(const T &f, const T &df,
                                      std::chars_format fmt = std::chars_format::general,
                                      int error_digits = 2, bool parenthesis = true)
    {
        // check invalid input
        if (df <= T(0) || error_digits <= 0)
            return std::string();

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

        if (fmt == std::chars_format::general) {
            fmt = ((precision > n - 1) && (n >= -3)) ? std::chars_format::fixed
                                                     : std::chars_format::scientific;
        }

        if (parenthesis) {
            if (fmt == std::chars_format::fixed) {
                // convert x to the rounded f value
                x *= std::pow(T(10), n);
                dx *= std::pow(T(10), dn);

                // find required # of decimals
                int decimals = std::max(precision - n, 0);

                if (decimals) {
                    if (error_digits <= decimals) {
                        dx *= std::pow(T(10), decimals);
                        return print_with_err_impl(x, decimals, dx, 0, fmt, parenthesis);
                    } else {
                        return print_with_err_impl(x, decimals, dx, decimals, fmt, parenthesis);
                    }
                } else {
                    return print_with_err_impl(x, 0, dx, 0, fmt, parenthesis);
                }

            } else if (fmt == std::chars_format::scientific) {
                x *= std::pow(T(10), 1);
                // dx *= std::pow(T(10),dn+1-n);

                int decimals = std::max(precision - 1, 0);

                if (decimals) {
                    if (error_digits <= decimals) {
                        dx *= std::pow(T(10), error_digits);
                        std::string s = print_with_err_impl(x, decimals, dx, 0,
                                                            std::chars_format::fixed, parenthesis);
                        s.append("×10");
                        s.append(superscript(n - 1));
                        return s;
                    } else {
                        dx *= std::pow(T(10), error_digits - decimals);
                        std::string s = print_with_err_impl(x, decimals, dx, decimals,
                                                            std::chars_format::fixed, parenthesis);
                        s.append("×10");
                        s.append(superscript(n - 1));
                        return s;
                    }
                } else {
                    dx *= std::pow(T(10), dn + 1 - n);
                    std::string s =
                            print_with_err_impl(x, 0, dx, 0, std::chars_format::fixed, parenthesis);
                    s.append("×10");
                    s.append(superscript(n - 1));
                    return s;
                }
            }

        } else { // !parentheses

            if (fmt == std::chars_format::fixed) {
                // convert x,dx to the rounded f,df values
                x *= std::pow(T(10), n);
                dx *= std::pow(T(10), dn);

                // find required # of decimals
                int decimals = std::max(precision - n, 0);

                if (decimals) {
                    return print_with_err_impl(x, decimals, dx, decimals, fmt, parenthesis);
                } else {
                    return print_with_err_impl(x, 0, dx, 0, fmt, parenthesis);
                }
            } else if (fmt == std::chars_format::scientific) {
                x *= std::pow(T(10), 1);
                dx *= std::pow(T(10), dn + 1 - n);

                int decimals = std::max(precision - 1, 0);

                if (decimals) {
                    std::string s("(");
                    s += print_with_err_impl(x, decimals, dx, decimals, std::chars_format::fixed,
                                             parenthesis);
                    s.append(")×10");
                    s.append(superscript(n - 1));
                    return s;
                } else {
                    std::string s("(");
                    s += print_with_err_impl(x, 0, dx, 0, std::chars_format::fixed, parenthesis);
                    s.append(")×10");
                    s.append(superscript(n - 1));
                    return s;
                }
            }
        }

        return std::string();
    }
};

#endif // ERROR_FMT_H
