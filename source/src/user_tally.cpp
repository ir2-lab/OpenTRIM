#include "user_tally.h"
#include "ion.h"
#include <iostream>

void user_tally::init()
{
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
    data_ = ArrayNDd(bin_sizes);
}

std::vector<size_t> user_tally::get_bin(const ion &i) const
{
    int n = bin_codes.size();
    std::vector<size_t> idx(n); // same size as bin_sizes

    for (int j=0;j<n;++j){
        double pos = 0;
        switch (bin_codes[j]) {
        case cX:
            pos = i.pos()[0]; // ion x-position
            idx[j] = std::upper_bound(par_.x.begin(), par_.x.end(), pos) - par_.x.begin() - 1; // id of bin starting from 0
            break;
        case cY:
            pos = i.pos()[1]; // ion y-position
            idx[j] = std::upper_bound(par_.y.begin(), par_.y.end(), pos) - par_.y.begin() - 1; // id of bin starting from 0
            break;
        case cZ:
            pos = i.pos()[2]; // ion z-position
            idx[j] = std::upper_bound(par_.z.begin(), par_.z.end(), pos) - par_.z.begin() - 1; // id of bin starting from 0
            break;
        default:
            break;
        }
    }
    return idx;
}

// int user_tally::get_bin_old(double xion) const
// {
//     auto binid = std::upper_bound(x.begin(), x.end(), xion) - x.begin() - 1;
//     // auto bin_bound = std::upper_bound(x.begin(), x.end(), xion);
//     // auto binit = std::find(x.begin(), x.end(), *bin_bound); //bin bound iterator
//     // int binid = std::distance(x.begin(), binit) - 1; // id of bin starting from 0
//     return binid;
// }

void user_tally::operator()(Event ev, const ion &i, const void *pv)
{
    if (ev != Event::IonStop)
        return;
    auto idx = get_bin(i);


    data_(idx)++;

}
































