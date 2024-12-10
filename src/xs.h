#ifndef _XS_H_
#define _XS_H_

#include <cmath>
#include <vector>
#include <array>
#include <cassert>

#define BOHR_RADIUS 0.05291772108 /* Bohr radius [nm] */
#define SCREENCONST 0.88534 /* Lindhard screening length constant */
#define E2C2 1.43996445      /* e^2 / 4 pi eps0 = e^2 c^2 in [eV][nm] */
#define AMUbyE 1.036426883E-8 /* amu/e = 1.660538782E-27/1.602176487E-19 */
#define ELEMENTARY_CHARGE 1.602176487E-19

/**
 * \defgroup XS Screened Coulomb scattering calculations
 * @{
 *
 * A set of C++ objects for classical scattering calculations of screened
 * Coulomb interatomic potentials.
 *
 * The general form of the potential is
 * \f[
 * V(r) = \frac{Z_1 Z_2 e^2}{r} \Phi(r/a)
 * \f]
 * where \f$ \Phi \f$ is the screening function and \f$ a \f$ the screening length.
 *
 * Different types of screening functions are defined
 * by the \ref screening_function templated structure with the enum class \ref Screening
 * as template parameter. The most used screening potentials are defined: 
 * - Ziegler-Biersack-Littmark (ZBL) Universal potential, Screening::ZBL
 * - Lenz-Jensen, Screening::LenzJensen
 * - Kr-C, Screening::KrC
 * - Moliere, Screening::Moliere
 *
 * The scattering angle in the center-of-mass system can be obtained by
 * the scattering integral
 * \f[
 * \theta = \pi - 2 s \int_{x_0}^\infty {x^{-2}F^{-1/2}(x)\,dx}
 * \f]
 * where
 * \f[
 * F(x) = 1 - \frac{\Phi(x)}{x\,\epsilon} - \frac{s^2}{x^2}
 * \f]
 * and \f$ x_0 \f$ is the distance of closest approach which satisfies
 * \f$ F(x_0)=0 \f$.
 *
 * \f$  \epsilon = E_{CM} a/Z_1 Z_2 e^2\f$ and \f$ s = p/a\f$ are the
 * reduced center-of-mass energy and impact parameter.
 *
 * The integral is evaluated by quadrature in the
 * \ref xs_quad template class, where the
 * Screening enum is again the template parameter.
 *
 * For comparison, the so-called MAGIC approximation of Biersack-Haggmark
 * for the scattering integrals of
 * the ZBL potential is also implemented
 * in the class \ref xs_zbl_magic.
 * 
 * As quadrature is a costly operation, the scattering integrals are typically tabulated
 * for use in Monte-Carlo codes.
 * Here we adapt the tabulation method of the program Corteo, see \ref CorteoIdx.
 *
 * The two classes \ref xs_zbl_corteo4bit and \ref xs_zbl_corteo6bit provide access to
 * pre-calculated tables of \f$ \sin^2 \theta/2 \f$ for the ZBL potential
 * as a function of
 * \f$  \epsilon \f$ and \f$ s \f$. The tables are compiled into dynamic libraries
 * and can be linked for use in a simulation program.
 *
 * For scattering calculations in the lab system, the \ref abstract_xs_lab class defines
 * the interface for all cross-section classes.
 *
 * The \ref xs_lab class template provides an implementation of a lab system cross-section
 * object.
 *
 * \ref xs_lab_zbl_corteo4bit and  \ref xs_lab_zbl_corteo6bit use the pre-computed tables for the
 * center-of-mass scattering angle to simulate scattering in the lab system.
 *
 * Ref.: Yuan et al. NIMB1993
 */

/**
 * @brief Screening function type enumeration
 */
enum class Screening {
    None = 0,       /**< Unscreened Coulomb potential */
    LenzJensen = 1, /**< Lenz-Jensen */
    KrC = 2,        /**< Kr-C */
    Moliere = 3,    /**< Moliere */
    ZBL = 4,        /**< Ziegler-Biersack-Littmark (ZBL) Universal */
    Invalid = -1
};

/**
 * @brief Screening function definition
 *
 * A templated structure that includes the definition of the screening
 * function (screening_function::operator()) and the screening length
 * (screening_function::screeningLength).
 *
 * Additionally, it returns the name and type of screening as a string or enum, respectively.
 *
 * @tparam ScreeningType (screening_function_t) the type of screening function
 */
template<Screening ScreeningType>
struct screening_function
{
    /**
     * @brief Screening length
     * @param Z1 is the atomic number of the projectile
     * @param Z2 is the atomic number of the target
     * @return the screening length [nm]
     */
    static double screeningLength(int Z1, int Z2);

    /**
     * @brief operator () implements the actual screening function
     * @param x is the radial distance in units of the screening length
     * @return the value of \f$ \Phi(x) \f$
     */
    double operator()(const double& x) const;

    /// The name of the screening function
    static const char* screeningName();

    /// The type of screening as a screening_function_t enum
    static Screening screeningType() { return ScreeningType; }
};

/**@}*/  // XS

// Explicit specialization for the unscreened Coulomb interaction
template<>
struct screening_function< Screening::None >
{
    static double screeningLength(int Z1, int Z2) { return BOHR_RADIUS; } // Bohr radius in nm
    double operator()(const double& x) const { return 1.; }
    static const char* screeningName() { return "Unscreened Coulomb"; }
    static Screening screeningType() { return Screening::None; }
};
// Explicit specialization for the Lenz-Jensen potential
template<>
struct screening_function< Screening::LenzJensen >
{
    static double screeningLength(int Z1, int Z2) {
        return SCREENCONST*BOHR_RADIUS/std::sqrt(std::pow(Z1,2./3)+std::pow(Z2, 2./3));
    }
    double operator()(const double& x) const {
        double y = 3.108*std::sqrt(x);
        return exp(-y)*(1.+y*(1.+y*(0.3344+y*(0.0485+2.647e-3*y))));
    }
    static const char* screeningName() { return "Lenz-Jensen"; }
    static Screening screeningType() { return Screening::LenzJensen; }
};
// Explicit specialization for the Kr-C potential
template<>
struct screening_function< Screening::KrC >
{
    static Screening screeningType() { return Screening::KrC; }
    static const char* screeningName() { return "Kr-C"; }
    static double screeningLength(int Z1, int Z2) {
        return SCREENCONST*BOHR_RADIUS/(std::pow(Z1,0.23)+std::pow(Z2,  0.23));
    }

    /* Screening function coefficients */
    constexpr static const double C[] =
        {0.190945, 0.473674, 0.335381};
    constexpr static const double A[] =
        {0.278544, 0.637174, 1.919249};


    double operator()(const double& x) const {
        return C[0]*exp(-A[0]*x)+C[1]*exp(-A[1]*x)+C[2]*exp(-A[2]*x);
    }
};
// Explicit specialization for the Moliere potential
template<>
struct screening_function< Screening::Moliere >
{
    static Screening screeningType() { return Screening::Moliere; }
    static const char* screeningName() { return "Moliere"; }
    static double screeningLength(int Z1, int Z2) {
        return SCREENCONST*BOHR_RADIUS/(std::pow(Z1,0.23)+std::pow(Z2,  0.23));
    }

    /* Screening function coefficients */
    constexpr static const double C[] =
        {0.35, 0.55, 0.10};
    constexpr static const double A[] =
        {0.30, 1.20, 6.00};

    double operator()(const double& x) const {
        return C[0]*exp(-A[0]*x)+C[1]*exp(-A[1]*x)+C[2]*exp(-A[2]*x);
    }
};
// Explicit specialization for the Ziegler-Biersack-Littmark (ZBL) potential
template<>
struct screening_function< Screening::ZBL >
{
    static Screening screeningType() { return Screening::ZBL; }
    static const char* screeningName() { return "Ziegler-Biersack-Littmark (ZBL)"; }
    static double screeningLength(int Z1, int Z2) {
        return SCREENCONST*BOHR_RADIUS/(std::pow(Z1,0.23)+std::pow(Z2,  0.23));
    }

    /* Universal screening function coefficients TRIM85 */
    constexpr static const double C[] =
        {0.18175, 0.50986, 0.28022, 0.028171};
    constexpr static const double A[] =
        {3.19980, 0.94229, 0.40290, 0.201620};

    double operator()(const double& x) const {
        return C[0]*exp(-A[0]*x)+
               C[1]*exp(-A[1]*x)+
               C[2]*exp(-A[2]*x)+
               C[3]*exp(-A[3]*x);
    }
};

/**
 * @brief The xs_base class forms the basis of all cross-section classes
 *
 * @tparam
 *   ScreeningType the type of screening function
 *
 * @ingroup XS
 */
template<Screening ScreeningType>
struct xs_base : public screening_function< ScreeningType >
{
    typedef screening_function< ScreeningType > Phi;

    /**
     * @brief The function \f$ F(x) \f$ under the scattering integral
     *
     * \f[
     * F(x) = 1 - \frac{\Phi(x)}{x\,e} - \frac{s^2}{x^2}
     * \f]
     *
     * @param x integral variable
     * @param e reduced energy
     * @param s reduced impact parameter
     * @return the value of the function
     */
    static double F(double x, double e, double s) {
        double sx = s/x;
        Phi p;
        return 1. - p(x)/(x*e)-sx*sx;
    }

    /**
     * @brief Finds the minimal approach distance of a scattered projectile
     *
     * For the unscreened Coulomb potential the minimal approach is
     * \f[
     * x_0 = \frac{1}{2\epsilon} \left[ 1 + \sqrt{1 + (2\,\epsilon\,s)^2} \right]
     * \f]
     *
     * For screened potentials \f$ x_0 \f$ is found by numerically solving
     * \f$ F(x_0)=0 \f$
     * using bysection.
     *
     * @param e reduced energy of the scattered particle
     * @param s reduced impact parameter
     * @return the minimal approach distance (in screening length units)
     */
    static double minApproach(double e, double s) {
        double x2 = 1.0/(2.0*e);
        x2 = x2+sqrt(x2*x2+s*s); // inital guesses: Mendenhall & Weller NIMB 58(1991)11, eq. 15

        double x1 = x2/10.;
        double f1 = F(x1, e, s);  // should be always negative
        double f2 = F(x2, e, s);  // should be always positive (starting ~1.0)

        // ensure bracketing
        while(f1 >=0. ) {
            // initial guess for x1 too optimistic
            x1 *= 0.1;
            f1 = F(x1, e, s);  // should be always negative
        }
        while(f2 <=0. ) {
            // just in case
            x2 *= 1.001;
            f2 = F(x2, e, s);
        }

        // assert(f1<0.0 && f2>0.0); // values should be on each side of 0

        double xm = 0.5*(x1+x2);
        double fm = F(xm, e, s);
        double d = 1.;
        while (std::abs(fm) > std::numeric_limits<double>::epsilon() &&
               std::abs(d) > std::numeric_limits<double>::epsilon()
               )
        {
            if (fm<0.) x1 = xm; else x2 = xm;
            double q = 0.5*(x1+x2);
            d = (q - xm)/xm;
            xm = q;
            fm = F(xm, e, s);
        }

        return xm;
    }

    /**
     * @brief Scattering angle in the Impulse approximation
     *
     * From Lehmann & Leibfried, Z. Physic 172 (1963) 465, the 1st order term
     * in the "momentum approx." for the screened potential \f$ V(r) = (A/r)e^{-r/a} \f$ is
     *
     * \f[
     * \theta_1 = \epsilon^{-1}\,K_1(s)
     * \f]
     *
     * where \f$ K_1(x) \f$ is the modified Hankel function (or modified
     * Bessel function of the 2nd kind).
     *
     * The above formula can be generalized for a screening function
     * expressed as a sum of exponentials, i.e., for the ZBL, Kr-C & Moliere potentials.
     * Namely, for a screening function \f$ \Phi(x) = \sum_i{C_i\, e^{-A_i x}} \f$ the scattering angle
     * in the impulse approx. is
     *
     * \f[
     * \theta_1 = \epsilon^{-1} \sum_i {C_i A_i K_1(A_i s) }
     * \f]
     *
     * In all other cases the function returns the unscreened Coulomb
     * exact scattering angle
     *
     * \f[
     * \theta = 2 \arcsin \left[ 1 + (2\epsilon\, s)^2\right]^{-1/2}
     * \f]
     *
     * @param e reduced energy of the scattered particle
     * @param s reduced impact factor
     * @return the scattering angle [rad]
     */
    static double theta_impulse_approx(double e, double s) {
        // exact unscreened Coulomb theta
        double x = 2*e*s;
        return 2*std::asin(std::sqrt(1./(1+x*x)));
    }

    // Modified bessel 2nd kind K1(x) with large x approx
    // Error for x>200 ~ below 1%
    // K1(200) ~ 1e-88
    static double bessel_k1(double x) {
        // sqrt(pi/2)
        constexpr static const double sqrtpihalf = 1.25331413731550012081;
        return (x > 200) ? sqrtpihalf*exp(-x)/std::sqrt(x) :
                   std::cyl_bessel_k(1,x);
    }
};

// Unscreened Coulomb minimal approach distance
template<>
inline double xs_base<Screening::None>::minApproach(double e, double s)
{
    double x0 = 1.0/(2*e);
    return x0+std::sqrt(x0*x0+s*s);
}
// ZBL potential impulse aprox
template<>
inline double xs_base<Screening::ZBL>::theta_impulse_approx(double e, double s)
{
    auto &C =  Phi::C;
    auto &A =  Phi::A;
    double th = C[0]*A[0]*bessel_k1(A[0]*s) +
                C[1]*A[1]*bessel_k1(A[1]*s) +
                C[2]*A[2]*bessel_k1(A[2]*s) +
                C[3]*A[3]*bessel_k1(A[3]*s);
    return th/e;
}
// KrC potential impulse aprox
template<>
inline double xs_base<Screening::KrC>::theta_impulse_approx(double e, double s)
{
    auto &C =  Phi::C;
    auto &A =  Phi::A;
    double th = C[0]*A[0]*bessel_k1(A[0]*s) +
                C[1]*A[1]*bessel_k1(A[1]*s) +
                C[2]*A[2]*bessel_k1(A[2]*s);
    return th/e;
}
// Moliere potential impulse aprox
template<>
inline double xs_base<Screening::Moliere>::theta_impulse_approx(double e, double s)
{
    auto &C =  Phi::C;
    auto &A =  Phi::A;
    double th = C[0]*A[0]*bessel_k1(A[0]*s) +
                C[1]*A[1]*bessel_k1(A[1]*s) +
                C[2]*A[2]*bessel_k1(A[2]*s);
    return th/e;
}

/**
 * @brief DE integration from Press et al. Numerical Recipes 3rd ed.
 */
template<class T>
class de_integrator {

    double a,b,hmax,s;
    T &func;
    int n;

    /*
         * On the ﬁrst call to the function next (n=1), the routine returns
         * the crudest estimate for the integral.
         * Subsequent calls to next (n=2,3,...) will improve the accuracy by adding
         * 2^(n-1) additional interior points.
         */
    double next() {
        double del,fact,q,qp1,sum,t,twoh;
        int it,j;
        n++;
        if (n == 1) {
            fact=0.25;
            return s=hmax*2.0*(b-a)*fact*func(0.5*(b+a),0.5*(b-a));
        } else {
            for (it=1,j=1;j<n-1;j++) it <<= 1;
            twoh=hmax/it;
            // Twice the spacing of the points to be added.
            t=0.5*twoh;
            for (sum=0.0,j=0;j<it;j++) {
                q=exp(-2.0*sinh(t));
                qp1 = 1.0 + q;
                del=(b-a)*q/qp1;
                fact=q/qp1/qp1*cosh(t);
                sum += fact*(func(a+del,del)+func(b-del,del));
                t += twoh;
            }
            return s=0.5*s+(b-a)*twoh*sum; // Replace s by its reﬁned value and return.
        }
    }

public:
    de_integrator(T &funcc, const double hmaxx=3.7)
        : func(funcc), hmax(hmaxx) {}

    double integrate(double aa, double bb, double eps = 1.e-10) {
        const int JMAX=20;
        double os=0.0;
        a=aa; b=bb; n=0;
        for (int j=0;j<JMAX;j++) {
            next();
            if (j > 5) // Avoid spurious early convergence.
                if (std::abs(s-os) < eps*std::abs(os) ||
                    (s == 0.0 && os == 0.0)) return s;
            os=s;
        }
        // too many iterations
        return std::numeric_limits<double>::quiet_NaN();
    }
};

/**
 * @brief The xs_quad class implements screened potential scattering integral evaluation by quadrature
 *
 * Using Gauss–Chebyshev quadrature (https://dlmf.nist.gov/3.5#v) the integral is evaluated as
 *
 * \f[
 *   \theta = \pi - \frac{2\,s}{x_0} \frac{\pi}{N}
 *   \sum_{j=0}^{N/2-1}{H\left[ \cos\left( \frac{\pi}{N}\,j + \frac{\pi}{2N} \right) \right]}
 * \f]
 * where
 * \f[
 * H(u) = \sqrt{\frac{1-u^2}{F(x_0/u)}}
 * \f]
 *
 * @tparam
 *   ScreeningType specifies the type of screening function
 *
 * @ingroup XS
 */
template<Screening ScreeningType = Screening::ZBL>
struct xs_quad : public xs_base< ScreeningType >
{
    using xs_base< ScreeningType >::F;
    using xs_base< ScreeningType >::minApproach;
    using xs_base< ScreeningType >::theta_impulse_approx;

    /**
     * @brief Return \f$ \sin^2(\theta/2) \f$
     * @param e the reduced energy
     * @param s the reduced impact parameter
     * @return the value of \f$ \sin^2(\theta/2) \f$
     */
    static double sin2Thetaby2(const double& e, const double& s)
    {
        double v = std::sin(0.5*theta(e,s));
        return v*v;
    }

    /**
     * @brief The function \f$ H(u) \f$ used in the quadrature sum
     *
     * \f[
     * H(u) = \sqrt{\frac{1-u^2}{F(x_0/u}}
     * \f]
     *
     * @param u function argument
     * @param x0 closest approach distance
     * @param e reduced energy
     * @param s reduced impact parameter
     * @return the value of H
     */
    static double H(double u, double x0, double e, double s) {
        return std::sqrt((1-u*u) / F(x0/u,e,s));
    }

    /**
     * @brief Scattering angle in center-of-mass (CM) system
     *
     * Calculated using Gauss-Chebyshev quadrature.
     *
     * For \f$ s\cdot \epsilon^{1/6} > 100 \f$ where the scattering angle is very small
     * (\f$ \theta < 10^{-8} \f$) this function
     * returns the impulse approx xs_base::theta_impulse_approx.
     *
     * The region of \f$ (\epsilon,s) \f$ where impulse approximation can
     * be applied has been found empirically.
     *
     * @param e reduced energy of the scattered particle in CM system
     * @param s reduced impact parameter
     * @param nsum number of terms in the Gauss-Mehler sum \f$ N \f$
     * @return
     */
    static double theta(double e, double s, int nsum = 100)
    {    
        // if s > 100/e^(1/6) use impulse approx
        // The limit was found empirically for theta < 1e-9
        double s3 = s*s*s;
        if (e*s3*s3 > 1.e12)
            return theta_impulse_approx(e,s);

        return M_PI - pi_minus_theta(e,s,nsum);
    }

    static double pi_minus_theta(double e, double s, int nsum = 100)
    {
        double x0 = minApproach(e, s);

        double sum(0.);
        int m = nsum >> 1;
        double a = M_PI/2/m;
        double b = a/2;
        for(unsigned int j=0; j<m; j++) {
            double uj = cos(a*j+b);
            sum += H(uj, x0, e, s);
        }

        return 2.0*s/x0*a*sum;
    }

    /**
     * @brief Return the reduced impact parameter
     *
     * Use bisection to find the reduced impact parameter s
     * given energy and scattering angle
     *
     * @param e reduced energy
     * @param thetaCM scattering angle (rad) in center-of-mass system
     * @return the reduced impact parameter
     */
    static double findS(double e, double thetaCM, double tol = std::numeric_limits<double>::epsilon()) {

        if (thetaCM==0.) return std::numeric_limits<double>::infinity();

        // inital guesses: Mendenhall & Weller NIMB 58 (1991) 11, eqs. 23-25
        double d = thetaCM / M_PI;
        double gamma = 1. - d;

        if (gamma==0.) return 0;

        double e0 = (d < 10*std::numeric_limits<double>::epsilon()) ?
                        2.*d*e : (1.0-gamma*gamma)*e;
        double x0 = minApproach(e0, 1.e-8);
        double x1, x2;
        if (e >= 1.) {
            x1 = 0.7*gamma*x0;
            x2 = 1.0/(2.0*e*tan(thetaCM/2.0));
        } else {
            x1 = 0.9*gamma*x0;
            x2 = 1.4*gamma*x0;
        }

        if (x2 > 1.e4) x2 = 1.e4; // this is as high as it gets. Above causes numerical instability

        // ensure bracketing
        while (thetaCM-theta(e, x1)>=0.0) x1 *= 0.1;
        while (thetaCM-theta(e, x2)<=0.0) x2 *= 1.001;

        assert(thetaCM-theta(e, x1)<0.0 && thetaCM-theta(e, x2)>0.0);    // values should be on each side of 0

        double xm = 0.5*(x1+x2);
        double fm = thetaCM-theta(e,xm);
        d = 1.;
        int k = 0;

        while (std::abs(d) > tol && k < 100)
        {
            if (fm < 0. ) x1 = xm;
            else x2 = xm;
            double q = 0.5*(x1+x2);
            d = (q - xm)/xm;
            xm = q;
            fm = thetaCM-theta(e,xm);
            k++;
        }

        return xm;
    }

    /**
     * @brief Differential cross-section in center-of-mass system
     *
     * \f[
     *   \frac{d\sigma}{d\Omega} = \frac{s}{\sin(\theta)}\left| \frac{ds}{d\theta}\right|
     * \f]
     *
     * In units of \f$ a^2 \f$
     *
     * @param e is the reduced energy
     * @param thetaCM is the center-of-mass scattering angle (rad)
     * @return the reduced cross-section
     */
    static double crossSection(double e, double thetaCM, double tol = std::numeric_limits<double>::epsilon())
    {
        // find corresponfding reduced impact parameter
        double s = findS(e, thetaCM, tol);

        if (s < 1.e-6) { // quasi head on collision
            double dth = pi_minus_theta(e,s);
            if (s==0.) {
                s = 1.e-30;
                dth = pi_minus_theta(e,s);
            }
            double ds = s*0.001;
            double dsdTheta = (12.0*ds)/(-pi_minus_theta(e,s+2.0*ds)
                                             +8.0*pi_minus_theta(e,s+ds)
                                             -8.0*pi_minus_theta(e,s-ds)
                                             +pi_minus_theta(e,s-2.0*ds));

            return s/sin(dth)*fabs(dsdTheta);
        }

        // ds/dTheta using five-point stencil
        double ds = s*0.001;
        double dsdTheta = (12.0*ds)/(-theta(e,s+2.0*ds)
                                         +8.0*theta(e,s+ds)
                                         -8.0*theta(e,s-ds)
                                         +theta(e,s-2.0*ds));


        return s/sin(thetaCM)*fabs(dsdTheta);
    }

private:
    struct eloss_functor_ {
        double e_;
        static double ergxs(double e, double mu) {
            double thetaCM = 2.*std::asin(std::sqrt(mu));
            return crossSection(e,thetaCM,1e-10)*mu;
        }
        double operator()(double x, double d) {
            return ergxs(e_, x);
        }
    };

public:

    /**
     * @brief Reduced stopping power
     *
     * Calculate the reduced stopping power
     *
     * \f[
     * S_n(\epsilon) = \frac{\epsilon}{\pi\, a^2 T_m N} \frac{dE}{dx} =
     * \frac{4 \epsilon}{a^2 T_m^2} \int_0^{T_m}{T\,\sigma(T) dT}
     * \f]
     *
     * where \f$ \sigma \f$ is the reduced cross-section.
     *
     * The integral is evaluated numerically.
     *
     * Optionally, the function calculates the stopping power
     * for scattering angles below `theta_max`. In this case the
     * upper limit in the integral is replaced with
     *
     * \f[
     * T(\theta_m) = T_m \, \sin^2 \theta_m/2
     * \f]
     *
     * @param e the reduced energy
     * @param theta_max optional maximum scattering angle, defaults to \f$ \pi \f$
     * @param tol optional rel. tolerance of the integration, default is 1e-6
     * @return the energy loss cross-section
     */
    static double stoppingPower(double e, double theta_max = M_PI, double tol = 1.e-6)
    {
        eloss_functor_ F;
        de_integrator<eloss_functor_> I(F);
        F.e_ = e;
        double mu_max = sin(theta_max/2);
        mu_max = mu_max*mu_max;
        return 4*e*I.integrate(0., mu_max, tol);
    }
};

/**
 * @brief Screened potential cross-section by the MAGIC formula
 *
 * Calculate the scattering integral for the scattering angle in
 * center-of-mass system and the cross-section for the
 * Ziegler-Biersack-Littmark (ZBL) Universal screening function
 * using the MAGIC interpolation formula of Biersack & Haggmark.
 *
 * Ref.: Biersack & Haggmark NIM1980
 *
 * @ingroup XS
 */
struct xs_zbl_magic : public xs_base< Screening::ZBL >
{
    using xs_base<Screening::ZBL>::minApproach;
    /**
     * @brief Scattering angle in center-of-mass (CM) system
     *
     * @param e reduced energy of the scattered particle in CM system
     * @param s reduced impact parameter
     * @return the scattering angle in rad
     */
    static double theta(double e, double s)
    {
        return 2*std::acos(MAGIC(e,s));
    }

    /**
     * @brief Return \f$ \sin^2(\theta/2) \f$
     * @param e the reduced energy
     * @param s the reduced impact parameter
     * @return the value of \f$ \sin^2(\theta/2) \f$
     */
    static double sin2Thetaby2(const double& e, const double& s)
    {
        double m = MAGIC(e,s);
        return 1. - m*m;
    }

    /**
     * @brief Implementation of MAGIC formula
     *
     * Returns \f$ \cos(\theta/2) \f$ where \f$ \theta \f$ is the center-of-mass
     * scattering angle
     *
     * @param e is the reduced energy in center of mass system
     * @param s is the reduced impact parameter
     * @return \f$ \cos(\theta/2) \f$
     */
    static double MAGIC(const double& e, const double& s)
    {
        double cost2;  /* cos(theta/2)*/
        double RoC,Delta,R,RR,A,G,alpha,beta,gamma,V,V1,FR,FR1,Q;
        double SQE;

        /* TRIM 85:  */
        static const double C[] = {0., 0.99229, 0.011615, 0.0071222, 14.813, 9.3066};

        /* Initial guess for R: */
        R=s;
        RR=-2.7*log(e*s);
        if(RR>=s){
            /*   if(RR<B) calc potential; */
            RR=-2.7*log(e*RR);
            if(RR>=s){
                R=RR;
            }
        }
        /* TRIM85: 330 */
        do{
            /* Calculate potential and its derivative */
            V = ZBL_and_deriv(R,&V1);
            FR  = s * s / R + V*R/e - R;
            FR1 = - s * s / (R * R) + (V+V1*R)/e - 1.0;
            Q   = FR/FR1;
            R   = R-Q;
        } while(fabs(Q/R)>0.001);

        RoC = -2.0 * (e-V)/V1;
        SQE = sqrt(e);

        alpha = 1+ C[1]/SQE;
        beta  = (C[2]+SQE) / (C[3]+SQE);           /* TRIM85: CC */
        gamma = (C[4]+e)/(C[5]+e);
        A     = 2*alpha*e*pow(s,beta);
        G     = gamma / ( sqrt((1.0+A*A))-A  );    /* TRIM85: 1/FF */
        Delta = A * (R-s)/(1+G);

        cost2=(s+RoC+Delta)/(R+RoC);
        return cost2;
    }

    /**
     * @brief ZBL potential
     * 
     * Evaluate the ZBL potential and optionally the 1st derivative dV/dR
     *
     * Implementation taken from ZBL85 
     * 
     * @param R is the reduced radius
     * @param Vprime if a non-NULL pointer is passed, it receives the value of dV/dR
     * @return the value of the potential
     */
    static double ZBL_and_deriv(double R, double* Vprime)
    {
        auto &C =  screening_function<Screening::ZBL>::C;
        auto &A =  screening_function<Screening::ZBL>::A;

        double EX1 = C[0]*exp(-A[0]*R);
        double EX2 = C[1]*exp(-A[1]*R);
        double EX3 = C[2]*exp(-A[2]*R);
        double EX4 = C[3]*exp(-A[3]*R);

        double V=(EX1+EX2+EX3+EX4)/R;
        if (Vprime)
            *Vprime = -(V + A[0]*EX1 + A[1]*EX2 + A[2]*EX3 + A[3]*EX4)/R;

        return V;
    }

    /**
     * @brief Return the reduced impact parameter
     *
     * Use bisection to find the reduced impact parameter s
     * given energy and scattering angle
     *
     * @param e reduced energy
     * @param thetaCM scattering angle (rad) in center-of-mass system
     * @return the reduced impact parameter
     */
    static double findS(double e, double thetaCM) {

        // inital guesses: Mendenhall & Weller NIMB 58 (1991) 11, eqs. 23-25
        double gamma = (M_PI-thetaCM)/M_PI;
        double x0 = minApproach((1.0-gamma*gamma)*e, 1.e-8);
        double x1 = 0.7*gamma*x0;
        double x2 = 1.0/(2.0*e*tan(thetaCM/2.0));

        // values for x1, x2 should be on either side of 0
        assert(thetaCM-theta(e, x1)<0.0 && thetaCM-theta(e, x2)>0.0);

        double xm = 0.5*(x1+x2);
        double fm = thetaCM-theta(e,xm);
        do {
            if (fm <0. ) x1 = xm;
            else x2 = xm;
            xm = 0.5*(x1+x2);
            fm = thetaCM-theta(e,xm);
        } while (fabs(fm)>1e-6);

        return xm;
    }

    /**
     * @brief Differential cross-section in center-of-mass system
     *
     * \f[
     *   \frac{d\sigma}{d\Omega} = \frac{s}{\sin(\theta)}\left| \frac{ds}{d\theta}\right|
     * \f]
     *
     * In units of \f$ a^2 \f$
     *
     * @param e is the reduced energy
     * @param thetaCM is the center-of-mass scattering angle (rad)
     * @return the reduced cross-section
     */
    static double crossSection(double e, double thetaCM)
    {
        // find corresponfding reduced impact parameter
        double s = findS(e, thetaCM);

        // ds/dTheta using five-point stencil
        double ds = s*0.001;
        double dsdTheta = (12.0*ds)/(-theta(e,s+2.0*ds)
                                         +8.0*theta(e,s+ds)
                                         -8.0*theta(e,s-ds)
                                         +theta(e,s-2.0*ds));


        return s/sin(thetaCM)*fabs(dsdTheta);
    }

    static double stoppingPower(double e, double theta_max = M_PI)
    {
        return 0.5*std::log(1+1.1383*e)/
               (e + 0.01321*std::pow(e,0.21226) +
                    0.19593*std::sqrt(e));
    }
};



/**
 * @brief The abstract_xs_lab class defines the interface for lab system cross-section objects
 *
 * @ingroup XS
 */
class abstract_xs_lab
{
protected:

    float screening_length_;    /* screening length [nm] */
    float mass_ratio_;          /* M1/M2 */
    float sqrt_mass_ratio_;     /* we will need this occasionally */
    float gamma_;           /* 4 M1 M2 / (M1 + M2)^2 */
    float red_E_conv_;          /* reduced energy conversion factor */
    float sig0_;                /* pi a^2 [nm^2] */

    template<class _XScm>
    void init_impl_(float Z1, float M1, float Z2, float M2)
    {
        /* Adapted from the corteo code */
        screening_length_ = _XScm::screeningLength(Z1,Z2);
        mass_ratio_       = M1/M2;
        sqrt_mass_ratio_  = std::sqrt(mass_ratio_);
        gamma_            = 4*mass_ratio_ / ((mass_ratio_+1) * (mass_ratio_+1));
        red_E_conv_       = screening_length_ / ((mass_ratio_+1) * Z1 * Z2 * E2C2);
        sig0_             = M_PI*screening_length_*screening_length_;
    }

public:
    virtual ~abstract_xs_lab() {}

    /// Returns the screening length \f$ a(Z_1,Z_2) \f$ in nm
    float screening_length() const { return screening_length_; }
    /// Returns the projectile to target mass ratio \f$ A = M_1 / M_2 \f$
    float mass_ratio() const { return mass_ratio_; }
    /// Returns the precomputed square root of the mass ratio
    float sqrt_mass_ratio() const { return sqrt_mass_ratio_; }
    /// Returns \f$ \gamma = 4 A / (1 + A)^2 \f$
    float gamma() const { return gamma_; }
    /// Return the reduced energy conversion factor \f$ f = a / (1+A)Z_1Z_2e^2  \f$ such that \f$ \epsilon = f\times E  \f$
    float red_E_conv() const { return red_E_conv_; }

    /**
     * @brief Initialize the cross-section object for a specific projectile-target combination
     * @param Z1 the projectile atomic number
     * @param M1 the projectile mass
     * @param Z2 the target atomic number
     * @param M2 the target mass
     */
    virtual void init(float Z1, float M1, float Z2, float M2) = 0;

    /**
     * @brief Calculate scattering angle and target recoil energy.
     *
     * Given the initial energy E and impact parameter S of an incoming
     * projectile, this function calculates the target recoil energy and the projectile
     * scattering angle sine and cosine.
     *
     * All quantities refer to the lab system.
     *
     * @param E is the initial projectile energy [eV]
     * @param S is the impact factor [nm]
     * @param recoil_erg is the target recoil energy [eV]
     * @param sintheta the sin of the scattering angle in the lab system
     * @param costheta the cos of the scattering angle in the lab system
     */
    virtual void scatter(float E, float S,
                         float &recoil_erg, float &sintheta, float &costheta) const = 0;

    /**
     * @brief Retuerns the impact parameter for given initial projectile energy and target recoil energy
     * @param E the projectile initial energy [eV]
     * @param T the target recoil energy [eV]
     * @return the corresponding impact factor [nm]
     */
    virtual float impactPar(float E, float T) const = 0;

    /**
     * @brief Differential cross-section \f$ d\sigma(E,T)/dT \f$
     *
     * \f[
     *   \frac{d\sigma}{dT}  = \frac{4\pi a^2}{\gamma E} \frac{d\sigma}{d\Omega_{CM}}
     * \f]
     *
     * @param E projectile energy [eV]
     * @param T recoil energy [eV]
     * @return the cross-section [nm^2/eV]
     */
    virtual float crossSection(float E, float T) const = 0;

    /**
     * @brief Stopping power \f$ S_n(E) = N^{-1}dE/dx \f$
     *
     * @param E projectile energy [eV]
     * @return the stopping power [eV-nm^2]
     */
    virtual float stoppingPower(float E) const = 0;

    /**
     * @brief Stopping power of collisions with recoil energy \f$ T \leq T_1 \f$
     *
     * @param E projectile energy [eV]
     * @param T1 maximum recoil energy [eV]
     * @return the stopping power [eV-nm^2]
     */
    virtual float stoppingPower(float E, float T1) const = 0;


};

/**
 * @brief The xs_lab class template describes a cross-section in lab system
 *
 * After initializing the class with XSlab::init for a given combination
 * of projectile and target, the function XSlab::scatter can be used to
 * calculate scattering quantities.
 *
 * @tparam
 *   xs_cm is the class of the center-of-mass cross-section
 *
 * @ingroup XS
 */
template<class xs_cm>
class xs_lab : public abstract_xs_lab
{
public:
    virtual void init(float Z1, float M1, float Z2, float M2) override
    {
        abstract_xs_lab::init_impl_<xs_cm>(Z1,M1,Z2,M2);
    }

    virtual void scatter(float E, float S,
                 float &recoil_erg, float &sintheta, float &costheta) const override
    {
        float e = E*red_E_conv_;
        float sin2thetaby2 = xs_cm::sin2Thetaby2(e, S/screening_length_);
        recoil_erg = E*gamma_*sin2thetaby2;
        /* convert scattering angle to lab frame of reference: */
        costheta = 1.f - 2*sin2thetaby2;
        sintheta = std::sqrt(1.f-costheta*costheta);
        float theta=atan( sintheta/(costheta + mass_ratio_) );
        sintheta=sin(theta);
        costheta=cos(theta);
    }

    virtual float impactPar(float E, float T) const override
    {
        double thetaCM = 1.0*T/E/gamma_;
        if (thetaCM > 1.0) return std::numeric_limits<float>::quiet_NaN();
        if (thetaCM == 1.0) return 0.f;
        thetaCM = 2.*std::asin(std::sqrt(thetaCM));
        return xs_cm::findS(E*red_E_conv_,thetaCM)*screening_length_;
    }

    virtual float crossSection(float E, float T) const override
    {
        double thetaCM = T/E/gamma_;
        if (thetaCM > 1.0) return std::numeric_limits<float>::quiet_NaN();
        thetaCM = 2.*std::asin(std::sqrt(thetaCM));
        return xs_cm::crossSection(E*red_E_conv_,thetaCM)*4*sig0_/E/gamma_;
    }

    virtual float stoppingPower(float E) const override
    {
        return xs_cm::stoppingPower(E*red_E_conv_)*sig0_*gamma_/red_E_conv_;
    }

    virtual float stoppingPower(float E, float T1) const override
    {
        double theta_max = 1.0*T1/E/gamma_;
        if (theta_max >= 1.) return stoppingPower(E);
        theta_max = 2.*std::asin(std::sqrt(theta_max));
        return xs_cm::stoppingPower(E*red_E_conv_,theta_max)*sig0_*gamma_/red_E_conv_;
    }
};

/**
 * @brief xs_lab implementation with ZBL potential and MAGIC formula for center-of-mass scattering angle
 *
 * @ingroup XS
 */
typedef xs_lab<xs_zbl_magic> xs_lab_zbl_magic;


#endif
