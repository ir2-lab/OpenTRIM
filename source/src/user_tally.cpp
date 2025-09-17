#include "user_tally.h"
#include "ion.h"
#include "target.h"

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

    push_bins(cAtom_id, par_.atom_id);

    // get bins from user options
    switch (par_.coordinates){
    case xyz:
        push_bins(cX, par_.x);
        push_bins(cY, par_.y);
        push_bins(cZ, par_.z);
        push_bins(cVX, par_.vx);
        push_bins(cVY, par_.vy);
        push_bins(cVZ, par_.vz);
        break;
    case cyl:
        push_bins(cRho, par_.rho);
        push_bins(cPhi, par_.phi);
        push_bins(cZ, par_.z);
        push_bins(cVRho, par_.vrho);
        push_bins(cVPhi, par_.vphi);
        push_bins(cVZ, par_.vz);
        break;
    case sph:
        push_bins(cR, par_.r);
        push_bins(cTheta, par_.theta);
        push_bins(cPhi, par_.phi);
        push_bins(cVR, par_.vr);
        push_bins(cVTheta, par_.vtheta);
        push_bins(cVPhi, par_.vphi);
        break;
    case Invalid:
        assert(false); // never get here!
        break;
    }

    idx.resize(bin_codes.size()); // same size as bin_sizes
    data_ = ArrayNDd(bin_sizes);    
}

// std::vector<std::string> user_tally::bin_names() const
void user_tally::bin_names(std::vector<std::string>& names, std::vector<std::string>& descriptions) const
{
    names.clear();
    descriptions.clear();
    for (int j = 0; j < bin_codes.size(); ++j){
        switch (bin_codes[j]) {
        case cX:
            names.push_back("x");
            descriptions.push_back("position vector x component [nm]");
            break;
        case cY:
            names.push_back("y");
            descriptions.push_back("position vector y component [nm]");
            break;
        case cZ:
            names.push_back("z");
            descriptions.push_back("position vector z component [nm]");
            break;
        case cRho:
            names.push_back("rho");
            descriptions.push_back("position vector rho component [nm???]");
            break;
        case cPhi:
            names.push_back("phi");
            descriptions.push_back("position vector phi component [rad???]");
            break;
        case cR:
            names.push_back("r");
            descriptions.push_back("position vector r component [nm???]");
            break;
        case cTheta:
            names.push_back("theta");
            descriptions.push_back("position vector theta component [rad???]");
            break;
        case cVX:
            names.push_back("vx");
            descriptions.push_back("direction vector x component");
            break;
        case cVY:
            names.push_back("vy");
            descriptions.push_back("direction vector y component");
            break;
        case cVZ:
            names.push_back("vz");
            descriptions.push_back("direction vector z component");
            break;
        case cVRho:
            names.push_back("vrho");
            descriptions.push_back("direction vector rho component");
            break;
        case cVPhi:
            names.push_back("vphi");
            descriptions.push_back("direction vector phi component");
            break;
        case cVR:
            names.push_back("vr");
            descriptions.push_back("direction vector r component");
            break;
        case cVTheta:
            names.push_back("vtheta");
            descriptions.push_back("direction vector theta component");
            break;
        case cAtom_id:
            names.push_back("atom_id");
            descriptions.push_back("atom_id");
            break;
        default:
            break;
        }
    }
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
        // int atom_id = i.myAtom()->id();

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
        case cAtom_id:{
            int atom_id = i.myAtom()->id();
            v = static_cast<float>(atom_id);
            break;
        }
        default:
            break;
        }

        idx[j] = std::upper_bound(bins[j].begin(), bins[j].end(), v) - bins[j].begin()
                - 1; // id of bin starting from 0
        if (idx[j] < 0 || idx[j] >= bin_sizes[j]) return false; // invalid index -> reject
    }

    return true;
}

bool user_tally::push_bins(variable_code c, const std::vector<float> &edges)
{
    if (edges.empty())
        return false;
    bin_codes.push_back(c);
    bins.push_back(edges);
    bin_sizes.push_back(edges.size() - 1);
    return true;
}

void user_tally::operator()(Event ev, const ion &i, const void *pv)
{
    if (ev != Event::IonStop)
        return;
    if (get_bin(i)) data_(idx)++;
}
































