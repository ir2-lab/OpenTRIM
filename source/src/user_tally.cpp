#include "user_tally.h"
#include "ion.h"
#include <iostream>

user_tally::user_tally() : bins(nbins, 0)
{
    // data_ = ArrayNDd(nbins);
    // // for (int i=0;i<nbins;i++)bins[i]=i*(xmax-xmin)/nbins;
    // auto i = bins.begin();
    // for (; i != bins.end(); i++)
    //     *i = (i - bins.begin()) * (xmax - xmin) / nbins;

    float factor = std::pow(xmax / xmin, 1.0f / nbins); // geometric spacing
    bins[0] = xmin;
    for (int i = 1; i <= nbins; ++i) {
        bins[i] = bins[i - 1] * factor;
    }
}

void user_tally::clear()
{
    std::fill(bins.begin(), bins.end(), 0);
}

int user_tally::get_bin(double x) const
{
    // if (x < xmin || x >= xmax) return -1;
    // int bin = static_cast<int>((x - xmin) / (xmax - xmin) * nbins);
    // return (bin >= 0 && bin < nbins) ? bin : -1;
    return std::upper_bound(bins.begin(), bins.end(), x) - bins.begin() - 1;
}

void user_tally::operator()(Event ev, const ion &i, const void *pv)
{
    if (ev != Event::IonStop)
        return;

    double x = i.pos()[0]; // ion x-position
    int bin = get_bin(x);
    if (bin >= 0)
        bins[bin]++;
    data_(bin) += 1;
}

void user_tally::print() const
{
    for (int i = 0; i < nbins; ++i)
        std::cout << "Bin " << i << ": " << bins[i] << "\n";
}
