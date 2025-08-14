#include "user_tally.h"
#include "ion.h"

void user_tally::init()
{
    // Init coord transformation (if needed)
    t.reset();
    if (par_.zaxis.size() || par_.xzvec.size() || par_.org.size())
        t.init(par_.zaxis, par_.xzvec, par_.org);

    // clear bins
    bin_codes.clear();
    bins.clear();
    bin_sizes.clear();

    // get bins from user options
    switch (par_.coordinates){
    case xyz:
        if (par_.x.size()){
            bin_codes.push_back(cX);
            bins.push_back(par_.x);
            bin_sizes.push_back(par_.x.size());
        }
        if (par_.y.size()){
            bin_codes.push_back(cY);
            bins.push_back(par_.y);
            bin_sizes.push_back(par_.y.size());
        }
        if (par_.z.size()){
            bin_codes.push_back(cZ);
            bins.push_back(par_.z);
            bin_sizes.push_back(par_.z.size());
        }
        if (par_.vx.size()){
            bin_codes.push_back(cVX);
            bins.push_back(par_.vx);
            bin_sizes.push_back(par_.vx.size());
        }
        if (par_.vy.size()){
            bin_codes.push_back(cVY);
            bins.push_back(par_.vy);
            bin_sizes.push_back(par_.vy.size());
        }
        if (par_.vz.size()){
            bin_codes.push_back(cVZ);
            bins.push_back(par_.vz);
            bin_sizes.push_back(par_.vz.size());
        }
        break;
    case cyl:
        if (par_.rho.size()){
            bin_codes.push_back(cRho);
            bins.push_back(par_.rho);
            bin_sizes.push_back(par_.rho.size());
        }
        if (par_.phi.size()){
            bin_codes.push_back(cPhi);
            bins.push_back(par_.phi);
            bin_sizes.push_back(par_.phi.size());
        }
        if (par_.z.size()){
            bin_codes.push_back(cZ);
            bins.push_back(par_.z);
            bin_sizes.push_back(par_.z.size());
        }
        if (par_.vrho.size()){
            bin_codes.push_back(cVRho);
            bins.push_back(par_.vrho);
            bin_sizes.push_back(par_.vrho.size());
        }
        if (par_.vphi.size()){
            bin_codes.push_back(cVPhi);
            bins.push_back(par_.vphi);
            bin_sizes.push_back(par_.vphi.size());
        }
        if (par_.vz.size()){
            bin_codes.push_back(cVZ);
            bins.push_back(par_.vz);
            bin_sizes.push_back(par_.vz.size());
        }
        break;
    case sph:
        if (par_.r.size()){
            bin_codes.push_back(cR);
            bins.push_back(par_.r);
            bin_sizes.push_back(par_.r.size());
        }
        if (par_.theta.size()){
            bin_codes.push_back(cTheta);
            bins.push_back(par_.theta);
            bin_sizes.push_back(par_.theta.size());
        }
        if (par_.phi.size()){
            bin_codes.push_back(cPhi);
            bins.push_back(par_.phi);
            bin_sizes.push_back(par_.phi.size());
        }
        if (par_.vr.size()){
            bin_codes.push_back(cVR);
            bins.push_back(par_.vr);
            bin_sizes.push_back(par_.vr.size());
        }
        if (par_.vtheta.size()){
            bin_codes.push_back(cVTheta);
            bins.push_back(par_.vtheta);
            bin_sizes.push_back(par_.vtheta.size());
        }
        if (par_.vphi.size()){
            bin_codes.push_back(cVPhi);
            bins.push_back(par_.vphi);
            bin_sizes.push_back(par_.vphi.size());
        }
        break;
    case Invalid:
        assert(false); // never get here!
        break;
    }

    idx.resize(bin_codes.size()); // same size as bin_sizes
    data_ = ArrayNDd(bin_sizes);

}

// std::vector<size_t> user_tally::get_bin(const ion &i) //const
bool user_tally::get_bin(const ion &i)
{
    int n = bin_codes.size();

    // get ion position at user tally ref. frame
    vector3 pos = t.transformPoint(i.pos());
    vector3 dir = t.transformVector(i.dir());

    for (int j = 0; j < n; ++j) {

        float v; // value for tally scoring

        switch (bin_codes[j]) {
        case cX:
            // pos = i.pos()[0]; // ion x-position
            v = pos.x(); // ion x-position
            break;
        case cY:
            // pos = i.pos()[1]; // ion y-position
            v = pos.y(); // ion y-position
            break;
        case cZ:
            // pos = i.pos()[2]; // ion z-position
            v = pos.z(); // ion z-position
            break;
        case cRho:
            // pos = sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)); // ion_rho = sqrt(x^2+y^2)
            v = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y()); // ion_rho = sqrt(x^2+y^2)
            break;
        case cPhi:
            // pos = std::atan2(i.pos()[1], i.pos()[0]); // ion_phi = atan(y/x)
            v = std::atan2(pos.y(), pos.x()); // ion_phi = atan(y/x)
            break;
        case cR:
            // pos = sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)+pow(i.pos()[2],2)); // ion_r = sqrt(x^2+y^2+z^2)
            v = pos.norm(); // ion_r = sqrt(x^2+y^2+z^2)
            break;
        case cTheta:
            // pos = std::atan2( sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)), i.pos()[2] ); // ion_theta = atan(sqrt(x^2+y^2)/z)
            v = std::atan2(std::sqrt(pos.x() * pos.x() + pos.y() * pos.y()),
                           pos.z()); // ion_theta = atan(sqrt(x^2+y^2)/z)
            break;
        case cVX:
            v = dir.x(); // ion x-velocity
            break;
        case cVY:
            v = dir.y(); // ion y-velocity
            break;
        case cVZ:
            v = dir.z(); // ion z-velocity
            break;
        case cVRho:
            v = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y()); // ion_vel_rho = sqrt(x^2+y^2)
            break;
        case cVPhi:
            v = std::atan2(dir.y(), dir.x()); // ion_vel_phi = atan(y/x)
            break;
        case cVR:
            v = dir.norm(); // ion_vel_r = sqrt(x^2+y^2+z^2)
            break;
        case cVTheta:
            v = std::atan2(std::sqrt(dir.x() * dir.x() + dir.y() * dir.y()),
                           dir.z()); // ion_vel_theta = atan(sqrt(x^2+y^2)/z)
            break;
        default:
            break;
        }

        idx[j] = std::upper_bound(bins[j].begin(), bins[j].end(), v) - bins[j].begin()
                - 1; // id of bin starting from 0
        if (idx[j] < 0 || idx[j] >= bin_sizes[j]) return false; // invalid index -> reject
    }

    return true;
}

void user_tally::operator()(Event ev, const ion &i, const void *pv)
{
    if (ev != Event::IonStop)
        return;
    if (get_bin(i)) data_(idx)++;
}
































