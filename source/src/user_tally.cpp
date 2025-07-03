#include "user_tally.h"
#include "ion.h"
#include <iostream>

void user_tally::init()
{
    data_ = ArrayNDd(nbins);
    bins.resize(nbins, 0);
    for (int i=0;i<nbins;i++)bins[i]=i*(xmax-xmin)/nbins;
    auto i = bins.begin();
    for (; i != bins.end(); i++)
        *i = (i - bins.begin()) * (xmax - xmin) / nbins;
    bins[0] = 0;
    bins[1] = 1;
    bins[2] = 2;
    bins[3] = 3;
    bins[4]= 3.5;
    bins[5] = 4;
    bins[6] = 5;
    bins[7] = 30;
}

// void user_tally::clear()
// {
//     data_.clear();
//     // std::fill(bins.begin(), bins.end(), 0);
//     // auto test1b=test1;
// }

int user_tally::get_bin(double x) const
{
    auto binid= std::upper_bound(bins.begin(), bins.end(), x) - bins.begin() - 1;
    // auto bin_bound = std::upper_bound(bins.begin(), bins.end(), x);
    // auto binit = std::find(bins.begin(), bins.end(), *bin_bound); //bin bound iterator
    // int binid = std::distance(bins.begin(), binit) - 1; // id of bin starting from 0
    return binid;
}

void user_tally::operator()(Event ev, const ion &i, const void *pv)
{
    if (ev != Event::IonStop)
        return;

    double x = i.pos()[0]; // ion x-position
    int bin = get_bin(x);

    if (bin >= 0) {
        data_(bin) += 1;
    }
}

void user_tally::print() const
{
    for (int i = 0; i < nbins; ++i)
        std::cout << "Bin " << i << ": " << bins[i] << "\n";
}
