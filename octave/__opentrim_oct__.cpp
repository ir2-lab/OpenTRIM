#include <octave/oct.h>

#include <screened_coulomb.h>
#include "dedx.h"

template <class Functor>
struct array_functor
{
    NDArray operator()(const NDArray &X, const NDArray &Y)
    {
        octave_idx_type nx = X.numel();
        octave_idx_type ny = Y.numel();
        Functor F;

        if (nx == ny) {
            NDArray Z(X);
            for (octave_idx_type i = 0; i < nx; i++)
                Z(i) = (std::isnan(X(i)) || std::isnan(Y(i)))
                        ? std::numeric_limits<double>::quiet_NaN()
                        : F(X(i), Y(i));
            return Z;
        }
        if (nx == 1) {
            NDArray Z(Y);
            for (octave_idx_type i = 0; i < ny; i++)
                Z(i) = (std::isnan(X(0)) || std::isnan(Y(i)))
                        ? std::numeric_limits<double>::quiet_NaN()
                        : F(X(0), Y(i));
            return Z;
        }
        if (ny == 1) {
            NDArray Z(X);
            for (octave_idx_type i = 0; i < nx; i++)
                Z(i) = (std::isnan(X(i)) || std::isnan(Y(0)))
                        ? std::numeric_limits<double>::quiet_NaN()
                        : F(X(i), Y(0));
            return Z;
        }
        NDArray Z;
        return Z;
    }
};

template <Screening ScreeningType>
struct theta_functor
{
    double operator()(double x, double y) { return xs_cms<ScreeningType>::theta(x, y); }
};

template <Screening ScreeningType>
struct ip_functor
{
    double operator()(double x, double y) { return xs_cms<ScreeningType>::find_s(x, y); }
};

template <Screening ScreeningType>
struct xs_functor
{
    double operator()(double x, double y) { return xs_cms<ScreeningType>::crossSection(x, y); }
};

template <Screening ScreeningType>
struct sn_functor
{
    double operator()(double x, double y) { return xs_cms<ScreeningType>::sn(x, y); }
};

DEFUN_DLD(__screened_coulomb_theta__, args, nargout, "Do not call directly")
{
    NDArray X = args(0).array_value();
    NDArray Y = args(1).array_value();
    Screening S = Screening(args(2).scalar_value());
    NDArray C;
    switch (S) {
    case Screening::None: {
        array_functor<theta_functor<Screening::None>> F;
        C = F(X, Y);
    } break;
    case Screening::Bohr: {
        array_functor<theta_functor<Screening::Bohr>> F;
        C = F(X, Y);
    } break;
    case Screening::KrC: {
        array_functor<theta_functor<Screening::KrC>> F;
        C = F(X, Y);
    } break;
    case Screening::Moliere: {
        array_functor<theta_functor<Screening::Moliere>> F;
        C = F(X, Y);
    } break;
    case Screening::ZBL: {
        array_functor<theta_functor<Screening::ZBL>> F;
        C = F(X, Y);
    } break;
    }
    return octave_value(C);
}
DEFUN_DLD(__screened_coulomb_ip__, args, nargout, "Do not call directly")
{
    NDArray X = args(0).array_value();
    NDArray Y = args(1).array_value();
    Screening S = Screening(args(2).scalar_value());
    NDArray C;
    switch (S) {
    case Screening::None: {
        array_functor<ip_functor<Screening::None>> F;
        C = F(X, Y);
    } break;
    case Screening::Bohr: {
        array_functor<ip_functor<Screening::Bohr>> F;
        C = F(X, Y);
    } break;
    case Screening::KrC: {
        array_functor<ip_functor<Screening::KrC>> F;
        C = F(X, Y);
    } break;
    case Screening::Moliere: {
        array_functor<ip_functor<Screening::Moliere>> F;
        C = F(X, Y);
    } break;
    case Screening::ZBL: {
        array_functor<ip_functor<Screening::ZBL>> F;
        C = F(X, Y);
    } break;
    }
    return octave_value(C);
}
DEFUN_DLD(__screened_coulomb_xs__, args, nargout, "Do not call directly")
{
    NDArray X = args(0).array_value();
    NDArray Y = args(1).array_value();
    Screening S = Screening(args(2).scalar_value());
    NDArray C;
    switch (S) {
    case Screening::None: {
        array_functor<xs_functor<Screening::None>> F;
        C = F(X, Y);
    } break;
    case Screening::Bohr: {
        array_functor<xs_functor<Screening::Bohr>> F;
        C = F(X, Y);
    } break;
    case Screening::KrC: {
        array_functor<xs_functor<Screening::KrC>> F;
        C = F(X, Y);
    } break;
    case Screening::Moliere: {
        array_functor<xs_functor<Screening::Moliere>> F;
        C = F(X, Y);
    } break;
    case Screening::ZBL: {
        array_functor<xs_functor<Screening::ZBL>> F;
        C = F(X, Y);
    } break;
    }
    return octave_value(C);
}
DEFUN_DLD(__screened_coulomb_sn__, args, nargout, "Do not call directly")
{
    NDArray X = args(0).array_value();
    NDArray Y = args(1).array_value();
    Screening S = Screening(args(2).scalar_value());
    NDArray C;
    switch (S) {
    case Screening::None: {
        array_functor<sn_functor<Screening::None>> F;
        C = F(X, Y);
    } break;
    case Screening::Bohr: {
        array_functor<sn_functor<Screening::Bohr>> F;
        C = F(X, Y);
    } break;
    case Screening::KrC: {
        array_functor<sn_functor<Screening::KrC>> F;
        C = F(X, Y);
    } break;
    case Screening::Moliere: {
        array_functor<sn_functor<Screening::Moliere>> F;
        C = F(X, Y);
    } break;
    case Screening::ZBL: {
        array_functor<sn_functor<Screening::ZBL>> F;
        C = F(X, Y);
    } break;
    }
    return octave_value(C);
}

DEFUN_DLD(__dedx__, args, nargout, "Returns raw corteo tables of electronic energy loss")
{
    int Z1 = args(0).int_value();
    NDArray Z2 = args(1).array_value();
    NDArray X2 = args(2).array_value();
    float N = args(3).double_value();
    int m = args(4).int_value();

    int n2 = Z2.numel();
    std::vector<int> z2(n2);
    std::vector<float> x2(n2);
    for (int i = 0; i < n2; ++i) {
        z2[i] = Z2(i);
        x2[i] = X2(i);
    }
    StoppingModel model = static_cast<StoppingModel>(m);
    dedx_interp interp(model, Z1, z2, x2, N);

    dedx_erange i;
    octave_idx_type n = i.size;
    RowVector e(n), d(n);
    const float *p = interp.data();
    for (octave_idx_type k = 0; k < n; k++) {
        e(k) = *i++;
        d(k) = *p++;
    }

    octave_value_list retval(2);
    retval(0) = e;
    retval(1) = d;
    return retval;
}
