#include "xs_corteo.h"

#include <iomanip>
#include <iostream>
#include <chrono>
#include <random>

/** \file */

using namespace std::chrono;
using namespace std;

#define NITER 1000000

template <Screening S_, Quadrature Q_, int N>
void test1(size_t M);

template <Screening S_>
int test2(size_t M);

int test3(size_t M);

int main()
{

    const char *sep = "\n==========================================================\n\n";

    test1<Screening::ZBL, Quadrature::GaussChebyshev, 16>(NITER);

    cout << sep;
    test1<Screening::ZBL, Quadrature::Lobatto4, 4>(NITER);

    cout << sep;
    test1<Screening::ZBL, Quadrature::Magic, 0>(NITER);

    cout << sep;
    test2<Screening::ZBL>(NITER);

    cout << sep;
    test3(NITER);

    return 0;
}

template <Screening S_, Quadrature Q_, int N>
void test1(size_t M)
{

    typedef xs_cms<S_, Q_, N> XS;

    cout << "TEST-1" << endl;
    cout << "Timing benchmark" << endl;
    cout << "Screening:   " << XS::screeningName() << endl;
    cout << "Quadrature:  " << XS::quadratureName() << endl;
    cout << "Order:  " << XS::quadratureOrder() << endl;
    cout << "Iterations:  " << M << endl;
    cout << endl;

    // random_device rd; // Will be used to obtain a seed for the random number engine
    mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
    gen.seed();
    uniform_real_distribution<float> ue(-5.0f, 6.0f);
    uniform_real_distribution<float> us(-7.0f, std::log10(60.f));

    high_resolution_clock::time_point t1, t2;
    duration<double, std::nano> dt1, dt2;

    cout << "Running ... ";
    cout.flush();

    t1 = high_resolution_clock::now();

    double mu = 0.0;

    for (size_t i = 0; i < M; i++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        mu += XS::sin2Thetaby2(e, s);
    }

    t2 = high_resolution_clock::now();

    dt1 = t2 - t1;

    cout << "done." << endl;
    ;

    cout << "Timing rngs ... ";
    cout.flush();

    t1 = high_resolution_clock::now();

    double ss = 0;
    gen.seed();

    for (size_t i = 0; i < M; i++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        ss += e;
        ss += s;
    }

    t2 = high_resolution_clock::now();

    dt2 = t2 - t1;

    cout << "done." << endl << endl;

    cout << "<mu> = " << setprecision(9) << mu / M << endl;
    cout << setprecision(3);
    cout << "time per call (ns): " << (dt1.count() - dt2.count()) / M << " ns" << std::endl;
}

template <Screening S_>
int test2(size_t M)
{

    corteo_xs_lab<S_> XS(1, 1, 1, 1);

    cout << "TEST-2" << endl;
    cout << "Timing benchmark for corteo tabulated XS" << endl;
    cout << "Screening:   " << XS.screeningName() << endl;
    cout << "Iterations:  " << M << endl;
    cout << endl;

    // random_device rd; // Will be used to obtain a seed for the random number engine
    mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
    gen.seed();
    uniform_real_distribution<float> ue(-5.0f, 6.0f);
    uniform_real_distribution<float> us(-7.0f, std::log10(60.f));

    high_resolution_clock::time_point t1, t2;
    duration<double, std::nano> dt1, dt2;

    cout << "Running ... ";
    cout.flush();

    t1 = high_resolution_clock::now();

    double mu = 0.0;

    for (size_t k = 0; k < M; k++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        float x = XS.sin2Thetaby2(e, s);
        mu += x;
    }

    t2 = high_resolution_clock::now();

    dt1 = t2 - t1;

    cout << "done." << endl;
    ;

    cout << "Timing rngs ... ";
    cout.flush();

    t1 = high_resolution_clock::now();

    double ss = 0;
    gen.seed();

    for (size_t i = 0; i < M; i++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        ss += e;
        ss += s;
    }

    t2 = high_resolution_clock::now();

    dt2 = t2 - t1;

    cout << "done." << endl << endl;

    cout << "<mu> = " << setprecision(9) << mu / M << endl;
    cout << setprecision(3);
    cout << "time per call (ns): " << (dt1.count() - dt2.count()) / M << " ns" << std::endl;

    return 0;
}

int test3(size_t M)
{

    corteo_xs_lab<Screening::ZBL> XS(1, 1, 1, 1);

    cout << "TEST-3" << endl;
    cout << "Timing benchmark to compare corteo_xs_lab::scatter/scatter2" << endl;
    cout << "Screening:   " << XS.screeningName() << endl;
    cout << "Iterations:  " << M << endl;
    cout << endl;

    // random_device rd; // Will be used to obtain a seed for the random number engine
    mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
    gen.seed();
    uniform_real_distribution<float> ue(0, 6.0f);
    uniform_real_distribution<float> us(-7.0f, std::log10(60.f));

    high_resolution_clock::time_point t1, t2;
    duration<double, std::nano> dt1, dt2, dt3;

    cout << "Running ... ";
    cout.flush();

    t1 = high_resolution_clock::now();

    double T1{ 0.0 }, S1{ 0.0 }, C1{ 0.0 };

    for (size_t k = 0; k < M; k++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        float t, ss, c;
        XS.scatter(e, s, t, ss, c);
        T1 += t;
        S1 += ss;
        C1 += c;
    }

    t2 = high_resolution_clock::now();

    dt1 = t2 - t1;

    t1 = high_resolution_clock::now();

    double T2{ 0.0 }, S2{ 0.0 }, C2{ 0.0 };
    gen.seed();

    for (size_t k = 0; k < M; k++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        float t, ss, c;
        XS.scatter2(e, s, t, ss, c);
        T2 += t;
        S2 += ss;
        C2 += c;
    }

    t2 = high_resolution_clock::now();

    dt2 = t2 - t1;

    cout << "done." << endl;

    cout << "Timing rngs ... ";
    cout.flush();

    t1 = high_resolution_clock::now();

    double ss = 0;
    gen.seed();

    for (size_t i = 0; i < M; i++) {
        float e = std::pow(10.0, ue(gen));
        float s = std::pow(10.0, us(gen));
        ss += e;
        ss += s;
    }

    t2 = high_resolution_clock::now();

    dt3 = t2 - t1;

    cout << "done." << endl << endl;

    int w1 = 12;
    cout << setprecision(9);
    cout << "<T1, sinΘ1, cosΘ1> = ";
    cout << setw(w1) << T1 / M << ' ';
    cout << setw(w1) << S1 / M << ' ';
    cout << setw(w1) << C1 / M << endl;
    cout << "<T2, sinΘ2, cosΘ2> = ";
    cout << setw(w1) << T2 / M << ' ';
    cout << setw(w1) << S2 / M << ' ';
    cout << setw(w1) << C2 / M << endl;
    cout << setprecision(3);
    cout << "time per call 1 : " << (dt1.count() - dt3.count()) / M << " ns" << std::endl;
    cout << "time per call 2 : " << (dt2.count() - dt3.count()) / M << " ns" << std::endl;
    return 0;
}
