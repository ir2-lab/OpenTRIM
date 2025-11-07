#include "user_tally.h"
#include "ion.h"
#include "target.h"
#include "event_stream.h"

void user_tally::init(size_t natoms)
{
    // Init coord transformation (if needed)
    par_.coordinate_system.init();

    // clear bins
    bin_codes.clear();
    bins.clear();
    bin_sizes.clear();

    // push all bins defined by user
    push_bins(cX, par_.x);
    push_bins(cY, par_.y);
    push_bins(cZ, par_.z);
    push_bins(cR, par_.r);
    push_bins(cRho, par_.rho);
    push_bins(cCosTheta, par_.cosTheta);
    push_bins(cNx, par_.nx);
    push_bins(cNy, par_.ny);
    push_bins(cNz, par_.nz);
    push_bins(cE, par_.E);
    push_bins(cTdam, par_.Tdam);
    push_bins(cV, par_.V, natoms);
    push_bins(cAtom_id, par_.atom_id);
    push_bins(cRecoil_id, par_.recoil_id);

    // prepare buffers to store data
    idx.resize(bin_codes.size()); // same size as bin_sizes
    data_ = ArrayNDd(bin_sizes);
}

// clang-format off
static const char *bin_name_[] = {
    "x", "y", "z",
    "r", "rho", "costheta",
    "nx", "ny", "nz",
    "E", "Tdam", "V",
    "atom_id", "recoil_id",
    "invalid_bin_name"    
};
static const char *bin_desc_[] = {
    "Position vector x component [nm]",
    "Position vector y component [nm]",
    "Position vector z component [nm]",
    "Radial distance r=sqrt(x^2+y^2+z^2) [nm]",
    "Cylindrical radial distance rho=sqrt(x^2+y^2) [nm]",
    "Cosine of polar angle cosTheta = z/r",
    "x-axis direction cosine",
    "y-axis direction cosine",
    "z-axis direction cosine",
    "Energy [eV]",
    "Damage energy [eV]",
    "Number of vacancies generated",
    "Atom id",
    "Recoil generation id",
    "invalid_bin_desc"    
};
// clang-format on

const char *user_tally::bin_name(int i) const
{
    size_t k = (i >= 0 && i < (int)bin_codes.size()) ? bin_codes[i] : cNumCodes;
    return bin_name_[k];
}

const char *user_tally::bin_desc(int i) const
{
    size_t k = (i >= 0 && i < (int)bin_codes.size()) ? bin_codes[i] : cNumCodes;
    return bin_desc_[k];
}

std::vector<std::string> user_tally::bin_names() const
{
    size_t m = bin_codes.size();
    std::vector<std::string> names(m);
    for (size_t i = 0; i < m; ++i)
        names[i] = bin_name_[bin_codes[i]];
    return names;
}

std::vector<std::string> user_tally::bin_descriptions() const
{
    size_t m = bin_codes.size();
    std::vector<std::string> d(m);
    for (size_t i = 0; i < m; ++i)
        d[i] = bin_desc_[bin_codes[i]];
    return d;
}

bool user_tally::get_bin(const ion &i, const void *pv)
{
    int n = bin_codes.size();

    // get ion position at user tally ref. frame
    vector3 pos = par_.coordinate_system.transformPoint(i.pos());
    vector3 dir = par_.coordinate_system.transformVector(i.dir());

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
        case cR:
            v = pos.norm(); // r = sqrt(x^2+y^2+z^2)
            break;
        case cRho:
            v = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y()); // rho = sqrt(x^2+y^2)
            break;
        case cCosTheta:
            v = pos.z() / pos.norm(); // cosTheta = z/r
            break;
        case cNx:
            v = dir.x(); // x-axis direction cosine
            break;
        case cNy:
            v = dir.y(); // y-axis direction cosine
            break;
        case cNz:
            v = dir.z(); // z-axis direction cosine
            break;
        case cE:
            v = (event() == Event::CascadeComplete)
                    ? reinterpret_cast<const pka_buffer *>(pv)->recoilE()
                    : i.erg();
            break;
        case cTdam:
            v = (event() == Event::CascadeComplete)
                    ? reinterpret_cast<const pka_buffer *>(pv)->Tdam()
                    : 0;
            break;
        case cV:
            v = (event() == Event::CascadeComplete)
                    ? reinterpret_cast<const pka_buffer *>(pv)->Vac(vac_id[j])
                    : 0;
            break;
        case cAtom_id:
            v = static_cast<float>(i.myAtom()->id());
            break;
        case cRecoil_id:
            v = static_cast<float>(i.recoil_id());
            break;
        default:
            break;
        }

        idx[j] = std::upper_bound(bins[j].begin(), bins[j].end(), v) - bins[j].begin()
                - 1; // id of bin starting from 0

        // valid bin index is  0 <= idx < size-1
        // (the last bin is not included by default)
        if (idx[j] < 0 || idx[j] >= bin_sizes[j] - 1)
            return false; // invalid index -> reject
    }

    return true;
}

bool user_tally::push_bins(variable_code c, const std::vector<float> &edges, size_t natoms)
{
    if (edges.empty())
        return false;
    if (c == cV) {
        // one bin for each target atom id=1,2,...natoms-1
        for (size_t i = 1; i < natoms; ++i) {
            bin_codes.push_back(c);
            bins.push_back(edges);
            bin_sizes.push_back(edges.size());
            vac_id.push_back(i - 1);
        }
    } else {
        bin_codes.push_back(c);
        bins.push_back(edges);
        bin_sizes.push_back(edges.size());
        vac_id.push_back(0);
    }
    return true;
}
