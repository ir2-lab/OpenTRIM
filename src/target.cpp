#include "target.h"
#include "xs.h"
#include "dedx.h"
#include "random_vars.h"

#include <sstream>
#include <stdexcept>

atom::atom(const parameters& p) :
    target_item(nullptr),
    id_(0), mat_(nullptr), p_(p)
{}


atom::atom(target *t, class material* m, int id) :
    target_item(t),
    id_(id), mat_(m)
{}

float atom::LSS_Tdam(float recoilE) const
{
    float x = lss_Efact_ * recoilE;
    x = x + 3.4008f * std::pow (x, 1.0f / 6.0f) + 0.40244 * std::pow (x, 0.75f);
    return recoilE / (1.f + lss_Kd_*x);
}
float atom::NRT(float Tdam) const
{
    if (Tdam < p_.Ed) return 0.f;
    float v = Tdam/nrt_L_;
    return (v < 1.f) ? 1.f : v;
}

material::material(const char* name) :
    target_item(nullptr), id_(0),
    name_(name)
{}

material::material(target *t, const char* name, int id) :
    target_item(t),
    id_(id), name_(name)
{}

atom* material::addAtom(const atom::parameters& p, float x)
{
    atom* a;
    if (target_) {
        a = new atom(target_, this, target_->atoms_.size());
        target_->atoms_.push_back(a);
    } else {
        a = new atom(nullptr, this, atoms_.size());
    }
    this->atoms_.push_back(a);
    a->p_ = p;
    a->X_ = x;
    X_.push_back(x);
    return a;
}

void material::init()
{
    cumX_.resize(atoms_.size());
    // prepare concentrations
    float tot = 0;
    for(int i=0; i<atoms_.size(); i++) tot += X_[i];
    meanZ_ = 0;
    meanM_ = 0;
    for(int i=0; i<atoms_.size(); i++) {
        X_[i] /= tot;
        atoms_[i]->X_ /= tot;
        meanZ_ += X_[i]*atoms_[i]->Z();
        meanM_ += X_[i]*atoms_[i]->M();
    }

    // make cumulative (for random sel)
    cumX_[0] = X_[0];
    for(int i=1; i<atoms_.size(); i++)
        cumX_[i] += X_[i] + cumX_[i-1];

    /*
     * For testing purposes and comparisons for SRIM,
     * we calculate the mean screening length and
     * energy reduction factor (as in ZBL85)
     */
    if (target_) {
        int ionZ = target_->projectile()->Z();
        float ionM = target_->projectile()->M();
        meanA_ = screening_function<Screening::ZBL>::screeningLength(ionZ, meanZ_); // nm
        meanF_ = meanA_ * meanM_ / ( ionZ * meanZ_ * (ionM + meanM_) * E2C2 );
        meanMinRedTransfer_ = dedx_index::minVal * meanF_ * (ionM + meanM_)*(ionM + meanM_) / (4*ionM*meanM_) ;
    }

    static const float AvogadroNum = 6.02214076e2f; // note the nm^3
    if (atomicDensityNM_ <= 0) {
        atomicDensityNM_ = AvogadroNum * massDensity_ / meanM_;
    } else if (massDensity_ <= 0) {
        massDensity_ = atomicDensityNM_ * meanM_ / AvogadroNum;
    }

    atomicRadius_ = 1.0/std::pow(4.0*M_PI*atomicDensityNM_/3, 1.0/3);
    layerDistance_ = 1.0/std::pow(atomicDensityNM_, 1.0/3);
    sqrtAtomicDistance_ = std::sqrt(atomicRadius_);
    sqrtRecFlDensity_   = 1.0 / std::sqrt(M_PI * atomicDensityNM_); // TODO  x 1 / sqrt(flight_length_constant)

    meanImpactPar_ = 1.0 / std::sqrt(M_PI*atomicDensityNM_*atomicRadius_);

    // LSS & NRT
    lss_Kd_ = 0.1334f * std::pow (1.f*meanZ_, 2.0f / 3.0f ) / std::sqrt( meanM_ );
    lss_Efact_ = 0.01014f * std::pow (1.f*meanZ_, -7.0f / 3.0f);
    // Ghoniem and Chou JNM1988 effective Ed
    nrt_Ed_ = 0;
    for(int i=0; i<atoms_.size(); i++) nrt_Ed_ += X_[i]/atoms_[i]->Ed();
    nrt_Ed_ = 1.f/nrt_Ed_;
    nrt_L_ = 5*nrt_Ed_/2;
    // atoms
    for(int i=0; i<atoms_.size(); i++) {
        atom* a = atoms_[i];
        a->lss_Kd_ = 0.1334f * std::pow (1.f*a->Z(), 2.0f / 3.0f ) / std::sqrt( a->M() );
        a->lss_Efact_ = 0.01014f * std::pow (1.f*a->Z(), -7.0f / 3.0f);
        a->nrt_L_ = 5*a->Ed()/2;
    }

}

material::material_desc_t material::getDescription() const
{
    material_desc_t md;
    md.density = massDensity_;
    md.isMassDensity = true;
    for(const atom* a : atoms_) {
        md.Z.push_back(a->Z());
        md.M.push_back(a->M());
        md.Ed.push_back(a->Ed());
        md.El.push_back(a->El());
        md.Es.push_back(a->Es());
        md.Er.push_back(a->Er());
    }
    md.X = X_;
    return md;
}

const atom* material::selectAtom(random_vars& g) const
{
    if (atoms_.size()==1) return atoms_.front();
    float u = g.u01s();
    int i=0;
    while((i < atoms_.size()-1) && (u > cumX_[i])) i++;
    return atoms_[i];
}

float material::LSS_Tdam(float recoilE) const
{
    float x = lss_Efact_ * recoilE;
    x = x + 3.4008f * std::pow (x, 1.0f / 6.0f) + 0.40244 * std::pow (x, 0.75f);
    return recoilE / (1.f + lss_Kd_*x);
}
float material::NRT(float Tdam) const
{
    if (Tdam < nrt_Ed_) return 0.f;
    float v = Tdam/nrt_L_;
    return (v < 1.f) ? 1.f : v;
}


target::target()
{
    // create the projectile ion placehoder
    atoms_.push_back(new atom(this,0,0));
}

target::~target()
{
    for(material* m : materials_) delete m;
    for(atom* a : atoms_) delete a;
}

void target::setProjectile(int Z, float M)
{
    atom* i = atoms_[0];
    i->p_.Z = Z;
    i->p_.M = M;
}

void target::init() {
    for(material* m : materials_) m->init();
}

material* target::addMaterial(const char* name)
{
    materials_.push_back(new material(this, name, materials_.size()));
    return materials_.back();
}

void target::fill(const box3D& box, const material* m)
{
    if (cells_.isNull() || cells_.size()!=grid_.ncells())
        cells_ = ArrayND<const material*>(grid_.x().size()-1, grid_.y().size()-1, grid_.z().size()-1);

    box3D realbox = grid_.box().intersection(box);
    box1D bx, by, bz;
    bx.min() = box1D::VectorType(realbox.min().x());
    bx.max() = box1D::VectorType(realbox.max().x());
    by.min() = box1D::VectorType(realbox.min().y());
    by.max() = box1D::VectorType(realbox.max().y());
    bz.min() = box1D::VectorType(realbox.min().z());
    bz.max() = box1D::VectorType(realbox.max().z());
    ibox1D rx = grid_.x().range(bx);
    ibox1D ry = grid_.y().range(by);
    ibox1D rz = grid_.z().range(bz);
    for(int i=rx.min().x(); i<=rx.max().x(); i++)
        for(int j=ry.min().x(); j<=ry.max().x(); j++)
            for(int k=rz.min().x(); k<=rz.max().x(); k++)
                cells_(i,j,k) = m;

    region rd;
    rd.material_id = m->name();
    rd.min = realbox.min();
    rd.max = realbox.max();
    regions_.push_back( rd );
}

target::target_desc_t target::getDescription() const
{
    target_desc_t td;

    for(const material* m : materials_)
        td.materials.push_back(m->getDescription());

    for(const region& rd : regions_)
        td.regions.push_back(rd);

    td.cell_count = {(int)grid_.x().size() - 1,
                     (int)grid_.y().size() - 1,
                     (int)grid_.z().size() - 1};

    td.cell_size = {grid_.x()[1] - grid_.x()[0],
                    grid_.y()[1] - grid_.y()[0],
                    grid_.z()[1] - grid_.z()[0]
    };

    td.periodic_bc = {grid_.x().periodic(),
                      grid_.y().periodic(),
                      grid_.z().periodic()};


    return td;
}

std::vector<std::string> target::atom_labels() const
{
    std::vector<std::string> lbls(atoms_.size());
    for(int i=0; i<atoms_.size(); i++) {
        std::string& s = lbls[i];
        const material* m = atoms_[i]->mat();
        s = atoms_[i]->name();
        if (m) {
            s += " in ";
            s += m->name();
        } else s += " ion";
    }  
    return lbls;  
}


