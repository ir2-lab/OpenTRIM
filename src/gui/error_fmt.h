#ifndef ERROR_FMT_H
#define ERROR_FMT_H

#include <cmath>
#include <cstring>
#include <string>

/*
 * Return x and exp so that
 *
 * num = x * 10^(*exp)
 *
 * with 0.1 <= |x| < 1
 *
 */
template<class T>
constexpr T frexp10 (const T& num, int* exp)
{
    if (num==T(0)) { // similar to frexp
        *exp = 0;
        return T(0);
    }
    T logn = std::log10(std::abs(num));
    *exp = std::floor(logn) + 1;
    return num * std::pow(T(10),-(*exp));
}

/*
 * Round a value according to its error
 *
 * f is a value with error df, round df to d significant
 * digits and then round f to the same order as
 * the error df.
 *
 * Default d=2. d=1 is also often used.
 *
 */
template<class T>
T round_with_err (const T& f, const T& df, int d=2)
{
    if (df<=0) return f;

    // split numbers to mantissa and exponent
    int n, dn;
    T  x = frexp10( f, &n);
    T dx = frexp10(df,&dn);

    // find the required precision = # of
    // significant digits to express
    // the value within error.
    // min precision is 1 (1 digit for the result)
    // round f to the precision
    int precision = std::max(n-dn+d, 1);
    T sc = std::pow(T(10), precision - n);
    return std::round(f*sc)/sc;
}

/**
 * @brief Print real value with error
 *
 * Prints the real value f and error df and returns as a std::string.
 *
 * The @a fmt option specifies fixed, scientific or general notation
 * similar to @ref std::printf().
 *
 * The value of @a d corresponds to the significant digits in the error @a df.
 * Typically @a d is equal to 2 (default) or 1.
 *
 * The @a parenthesis option selects between two modes of error reporting.
 *
 * Example:
 *
 * The value 0.0123 with error 0.0045 will be printed as
 *   - 0.012(5) with fmt='f' or'g', d=1, parenthesis=true
 *   - 0.012±0.005 with fmt='f' or'g', d=1, parenthesis=false
 *   - 1.23(45)*1e-2 with fmt='e', d=2, parenthesis=true
 *   - (1.23±0.45)*1e-2 with fmt='e', d=2, parenthesis=false
 *
 * @param f value to be printed
 * @param df error of f, also printed
 * @param fmt format specifier, 'f'-fixed, 'e'-scientific, 'g'-general
 * @param d # of significant digits in df
 * @param parenthesis use parenthesis style when printing the error
 * @return a std::string containing the printed value
 */
template<class T>
std::string print_with_err(const T& f, const T& df, char fmt = 'g', int d = 2, bool parenthesis = true)
{
    // check invalid input
    if (df<=T(0) || d<=0) return std::string();
    // check invalid fmt char, replace with 'g'
    static const char* fmt_chars = "fFeEgG";
    if (std::strchr(fmt_chars,fmt)==NULL) fmt = 'g';

    // split numbers to mantissa and exponent
    int n, dn;
    T  x = frexp10( f, &n);
    T dx = frexp10(df,&dn);

    // round error to the required digits d
    T sc =std::pow(T(10),d);
    dx = std::round(dx*sc)/sc;

    // find the required precision = # of
    // significant digits to express
    // the value + error.
    // min precision is 1 (1 digit for the result)
    // round f to the precision
    int precision = std::max(n-dn+d, 1);
    sc = std::pow(T(10),precision);
    x = std::round(x*sc)/sc;

    if (fmt=='g' || fmt=='G') {
        fmt = ((precision > n-1) && (n>=-3)) ? 'f' : 'e';
    }

    char buff[1024];

    if (parenthesis) {

        if (fmt=='f' || fmt=='F') {
            // convert x to the rounded f value
            x *= std::pow(T(10),n);
            dx *= std::pow(T(10),dn);

            // find required # of decimals
            int decimals = std::max(precision - n, 0);

            if (decimals) {
                if (d<=decimals) {
                    dx *= std::pow(T(10),decimals);
                    snprintf(buff, 1024, "%.*f(%.0f)", decimals, x, dx);
                } else {
                    snprintf(buff, 1024, "%.*f(%.*f)", decimals, x, decimals, dx);
                }
            } else {
                snprintf(buff, 1024, "%.0f(%.0f)", x, dx);
            }

            return std::string(buff);

        } else if (fmt=='e' || fmt=='E') {

            x *= std::pow(T(10),1);
            //dx *= std::pow(T(10),dn+1-n);

            int decimals = std::max(precision-1,0);

            if (decimals) {
                if (d<=decimals) {
                    dx *= std::pow(T(10),d);
                    snprintf(buff, 1024, "%.*f(%.0f)*1e%d", decimals, x, dx, n-1);
                } else {
                    dx *= std::pow(T(10),d-decimals);
                    snprintf(buff, 1024, "%.*f(%.*f)*1e%d", decimals, x, decimals, dx, n-1);
                }
            } else {
                dx *= std::pow(T(10),dn+1-n);
                snprintf(buff, 1024, "%.0f(%.0f)*1e%d", x, dx, n-1);
            }

            return std::string(buff);
        }
    } else { // !parentheses

        if (fmt=='f' || fmt=='F') {
            // convert x,dx to the rounded f,df values
            x *= std::pow(T(10),n);
            dx *= std::pow(T(10),dn);

            // find required # of decimals
            int decimals = std::max(precision - n, 0);

            if (decimals) {
                snprintf(buff, 1024, "%.*f±%.*f",
                         decimals, x, decimals, dx);
            } else {
                snprintf(buff, 1024, "%.0f±%.0f",x,dx);
            }

            return std::string(buff);

        } else if (fmt=='e' || fmt=='E') {
            x *= std::pow(T(10),1);
            dx *= std::pow(T(10),dn+1-n);

            int decimals = std::max(precision-1,0);

            if (decimals) {
                snprintf(buff, 1024, "(%.*f±%.*f)*1e%d",
                         decimals, x, decimals, dx, n-1);
            } else {
                snprintf(buff, 1024, "(%.0f±%.0f)*1e%d",x,dx,n-1);
            }

            return std::string(buff);
        }
    }

    return std::string();
}

#endif // ERROR_FMT_H
