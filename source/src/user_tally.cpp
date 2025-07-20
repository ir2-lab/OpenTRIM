#include "user_tally.h"
#include "ion.h"
#include <iostream>

void user_tally::init()
{
    if (par_.coordinates == "xyz"){
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
    }
    else if (par_.coordinates == "cyl"){
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
    }
    else if (par_.coordinates == "sph"){
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
    }

    // int n = bin_codes.size();
    idx.resize(bin_codes.size()); // same size as bin_sizes
    data_ = ArrayNDd(bin_sizes);
}

// std::vector<size_t> user_tally::get_bin(const ion &i) //const
bool user_tally::get_bin(const ion &i)
{
    int n = bin_codes.size();
    for (int j=0;j<n;++j){
        float pos;
        switch (bin_codes[j]) {
        case cX:
            pos = i.pos()[0]; // ion x-position
            // idx[j] = std::upper_bound(par_.x.begin(), par_.x.end(), pos) - par_.x.begin() - 1; // id of bin starting from 0
            break;
        case cY:
            pos = i.pos()[1]; // ion y-position
            // idx[j] = std::upper_bound(par_.y.begin(), par_.y.end(), pos) - par_.y.begin() - 1;
            break;
        case cZ:
            pos = i.pos()[2]; // ion z-position
            // idx[j] = std::upper_bound(par_.z.begin(), par_.z.end(), pos) - par_.z.begin() - 1;
            break;
        case cRho:
            pos = sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)); // ion_rho = sqrt(x^2+y^2)
            // idx[j] = std::upper_bound(par_.rho.begin(), par_.rho.end(), pos) - par_.rho.begin() - 1; // id of bin starting from 0
            break;
        case cPhi:
            // pos = atan(i.pos()[1]/i.pos()[0]); // ion_phi = atan(y/x)
            pos = std::atan2(i.pos()[1], i.pos()[0]); // ion_phi = atan(y/x)
            // idx[j] = std::upper_bound(par_.phi.begin(), par_.phi.end(), pos) - par_.phi.begin() - 1;
            break;
        case cR:
            pos = sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)+pow(i.pos()[2],2)); // ion_r = sqrt(x^2+y^2+z^2)
            // idx[j] = std::upper_bound(par_.r.begin(), par_.r.end(), pos) - par_.r.begin() - 1; // id of bin starting from 0
            break;
        case cTheta:
            // pos = atan( sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)) / i.pos()[2] ); // ion_theta = atan(sqrt(x^2+y^2)/z)
            pos = std::atan2( sqrt(pow(i.pos()[0],2)+pow(i.pos()[1],2)), i.pos()[2] ); // ion_theta = atan(sqrt(x^2+y^2)/z)
            // idx[j] = std::upper_bound(par_.theta.begin(), par_.theta.end(), pos) - par_.theta.begin() - 1;
            break;
        default:
            break;
        }
        idx[j] = std::upper_bound(bins[j].begin(), bins[j].end(), pos) - bins[j].begin() - 1;
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
































