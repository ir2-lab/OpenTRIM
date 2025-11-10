#include "ion_beam.h"
#include "random_vars.h"
#include "ion.h"
#include "target.h"

ion_beam::ion_beam() : par_() { }

void ion_beam::setParameters(const parameters &p)
{
    par_ = p;
}

void ion_beam::init(target &t, bool is_cascade_sim)
{
    t.setProjectile(par_.ion);
    is_pka_source_ = is_cascade_sim;

    par_.energy_distribution.init();
    par_.spatial_distribution.init(t);
    par_.angular_distribution.init(t);
}

void ion_beam::source_ion(random_vars &g, const target &t, ion &i)
{
    i.setGrid(&t.grid());

    vector3 v;
    par_.spatial_distribution.sample(g, t, v);
    i.setPos(v);

    if (is_pka_source_) {
        const material *mat = t.cell(i.cellid());
        i.setAtom(mat->selectAtom(g));
    } else
        i.setAtom(t.atoms().front());

    i.setErg(par_.energy_distribution.sample(g));

    par_.angular_distribution.sample(g, t, v);
    i.setNormalizedDir(v);

    i.setTime(0.0);
}

float ion_beam::energy_distribution_t::sample(random_vars &r) const
{
    float e;
    switch (type) {
    case SingleValue:
        return center;
    case Uniform:
        return a + b * r.u01s();
    case Gaussian:
        do {
            e = center + r.normal() * a;
        } while (e <= 0.f);
        return e;
    default:
        assert(0);
        break;
    }
    return 0;
}

void ion_beam::energy_distribution_t::init()
{
    switch (type) {
    case Uniform:
        a = std::max(0.f, center - fwhm / 2);
        b = center + fwhm / 2 - a;
        break;
    case Gaussian:
        a = fwhm / 2.354820045; // fwhm = 2*sqrt(2*ln(2))
        break;
    case SingleValue:
        break;
    default:
        assert(0);
        break;
    }
}

void ion_beam::spatial_distribution_t::sample(random_vars &g, const target &t, vector3 &pos) const
{
    switch (geometry) {
    case Surface:
        switch (type) {
        case SingleValue:
            pos = center;
            break;
        case Uniform:
            pos.x() = a.x();
            pos.y() = a.y() + b.y() * g.u01s();
            pos.z() = a.z() + b.z() * g.u01s();
            break;
        case Gaussian:
            do {
                pos.x() = t.grid().x().front();
                pos.y() = center.y() + sig * g.normal();
                pos.z() = center.z() + sig * g.normal();
            } while (!t.grid().box().contains(pos));
            break;
        default:
            assert(0);
            break;
        }
        break;
    case Volume:
        switch (type) {
        case SingleValue:
            pos = center;
            break;
        case Uniform:
            pos.x() = a.x() + b.x() * g.u01s();
            pos.y() = a.y() + b.y() * g.u01s();
            pos.z() = a.z() + b.z() * g.u01s();
            break;
        case Gaussian:
            do {
                pos.x() = center.x() + sig * g.normal();
                pos.y() = center.y() + sig * g.normal();
                pos.z() = center.z() + sig * g.normal();
            } while (!t.grid().box().contains(pos));
            break;
        default:
            assert(0);
            break;
        }
        break;
    default:
        assert(0);
        break;
    }
}

void ion_beam::spatial_distribution_t::init(const target &t)
{
    switch (geometry) {
    case Surface:
        switch (type) {
        case SingleValue:
            break;
        case Uniform:
            a.x() = t.grid().x().front();
            a.y() = std::max(t.grid().y().front(), center.y() - fwhm / 2);
            a.z() = std::max(t.grid().z().front(), center.z() - fwhm / 2);
            b.x() = 0;
            b.y() = std::min(t.grid().y().back(), center.y() + fwhm / 2) - a.y();
            b.z() = std::min(t.grid().z().back(), center.z() + fwhm / 2) - a.z();
            break;
        case Gaussian:
            sig = fwhm / 2.354820045; // fwhm = 2*sqrt(2*ln(2))*sig = 2.355*sig
            break;
        default:
            assert(0);
            break;
        }
        break;
    case Volume:
        switch (type) {
        case SingleValue:
            break;
        case Uniform:
            a.x() = std::max(t.grid().x().front(), center.x() - fwhm / 2);
            a.y() = std::max(t.grid().y().front(), center.y() - fwhm / 2);
            a.z() = std::max(t.grid().z().front(), center.z() - fwhm / 2);
            b.x() = std::min(t.grid().x().back(), center.x() + fwhm / 2) - a.x();
            b.y() = std::min(t.grid().y().back(), center.y() + fwhm / 2) - a.y();
            b.z() = std::min(t.grid().z().back(), center.z() + fwhm / 2) - a.z();
            break;
        case Gaussian:
            sig = fwhm / 2.354820045; // fwhm = 2*sqrt(2*ln(2))*sig = 2.355*sig
            break;
        default:
            assert(0);
            break;
        }
        break;
    default:
        assert(0);
        break;
    }
}

void ion_beam::angular_distribution_t::sample(random_vars &g, const target &t, vector3 &dir) const
{
    dir = norm_center;
    switch (type) {
    case SingleValue:
        break;
    case Uniform:
    case Gaussian: // TODO: Gaussian
    {
        float nx, ny, costh, sinth;
        g.random_azimuth_dir(nx, ny);
        costh = 1.f - g.u01s() * 2 * mu;
        sinth = std::sqrt(1 - costh * costh);
        dir = { nx * sinth, ny * sinth, costh };
        dir = cs.rotation().transpose() * dir;
    }
        dir.normalize();
        break;
    default:
        assert(0);
        break;
    }
}

void ion_beam::angular_distribution_t::init(const target &t)
{
    norm_center = center.normalized();
    mu = std::min(1.f, float(fwhm / 4 / M_PI));

    // define a CS with the z-axis parallel to the center of the beam
    cs.zaxis = norm_center;
    // get an arbitrary xz-plane vector - just not collinear with zaxis
    cs.xzvector = { 1, 0, 0 };
    if (cs.xzvector.cross(norm_center).isZero())
        cs.xzvector = { 0, 1, 0 };
    // should be ok, but just in case
    if (cs.xzvector.cross(norm_center).isZero())
        cs.xzvector = { 0, 0, 1 };
    cs.init();
}
