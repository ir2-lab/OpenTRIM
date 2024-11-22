#include "mcdriver.h"
#include "elements.h"

#include <thread>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cctype>

#define CHECK_EMPTY_VEC(MatDesc,V,MatName) \
if (MatDesc.V.size()==0) throw std::invalid_argument("Empty "#V" in " + MatName);

#define CHECK_VEC_SIZE(MatDesc,V,N,MatName) \
if (MatDesc.V.size()!=N) throw std::invalid_argument("In material descriptor " + MatName + " the # of values in "#V" is not equal to those of Z." );

#define ANY_ZEROorNEG(V) \
std::any_of(V.begin(),V.end(),[](float c){ return c<=0.f; })

#define CHECK_VEC_ZEROorNEG(MatDesc,V,MatName) \
    if (ANY_ZEROorNEG(MatDesc.V)) throw std::invalid_argument("In material descriptor " + MatName + " "#V" has zero or negative values." );


mcdriver::mcdriver() :
    s_(nullptr), ips_(0.)
{

}

mcdriver::~mcdriver()
{
    if (s_) delete s_;
}

std::string mcdriver::outFileName(const char* type, int thread_id)
{
    std::stringstream ss;
    ss << out_opts_.OutputFileBaseName << '.' << type;
    if (thread_id >= 0) ss << thread_id;
    return ss.str();
}

std::string mcdriver::outFileName() const
{
    std::string s(out_opts_.OutputFileBaseName);
    s += ".h5";
    return s;
}

void mcdriver::getOptions(options& opt) const
{
    opt.Driver = par_;
    opt.Output = out_opts_;
    opt.Simulation = s_->getSimulationParameters();
    opt.Transport = s_->getTransportOptions();
    opt.IonBeam = s_->getSource().getParameters();
    opt.Target = s_->getTarget().getDescription();
}

void mcdriver::setOptions(const options& o)
{
    if (s_) {
        delete s_;
        s_ = nullptr;
    }
    s_ = o.createSimulation();
    par_ = o.Driver;
    out_opts_ = o.Output;
    s_->init();
}

int mcdriver::exec(progress_callback cb, size_t msInterval, void *callback_user_data)
{
    using namespace std::chrono_literals;
    static const size_t msTick = 100;

    int nthreads = par_.threads;
    if (nthreads < 1) nthreads = 1;

    // TIMING
    start_time_ = std::time(nullptr);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start);

    // If ion_count == 0, i.e. simulation starts, seed the rng
    if (s_->ion_count() == 0) s_->seed(par_.seed);

    // create simulation clones
    std::vector< mccore* > sims(nthreads);
    for(int i=0; i<nthreads; i++) sims[i] = new mccore(*s_);

    // jump the rng's of clones (except the 1st one)
    for(int i=1; i<nthreads; i++) {
        for(int j=0; j<i; ++j) sims[i]->rngJump();
    }

    // open clone streams
    for(int i=0; i<nthreads; i++) {
        if (out_opts_.store_pka)
            sims[i]->open_pka_stream();
        if (out_opts_.store_transmitted_ions)
            sims[i]->open_exit_stream();
    }
    // If ion_count == 0, i.e. simulation starts,
    // open the main streams
    if (s_->ion_count() == 0) {
        if (out_opts_.store_pka) s_->open_pka_stream();
        if (out_opts_.store_transmitted_ions) s_->open_exit_stream();
    }

    // set max ions and seed in each thread
    // reset abort flag
    for(int i=0; i<nthreads; i++) {
        sims[i]->setMaxIons(par_.max_no_ions);
        sims[i]->clear_abort_flag();
    }

    // create & start worker threads
    std::vector< std::thread > thread_pool;
    for(int i=0; i<nthreads; i++)
        thread_pool.emplace_back(&mccore::run, sims[i]);


    // report progress if callback function is given
    if (cb) {
        thread_ion_count_.assign(nthreads,0);
        size_t iTick, nTick = std::max(msInterval / msTick, 1UL);

        do  {

            for(int i=0; i<nthreads; i++)
                thread_ion_count_[i] = sims[i]->thread_ion_count();
            ion_count_ = s_->ion_count();

            // consolidate results
            for(int i=0; i<nthreads; i++)
                s_->mergeTallies(*(sims[i]));

            cb(*this,callback_user_data);

            iTick = 0;
            while (iTick < nTick && !(s_->abort_flag())) {
                std::this_thread::sleep_for(std::chrono::milliseconds(msTick));
                iTick++;
            }

        } while(s_->ion_count() < par_.max_no_ions && !(s_->abort_flag()));
    }

    // wait for threads to finish...
    for(int i=0; i<nthreads; i++) thread_pool[i].join();

    // consolidate tallies & events
    for(int i=0; i<nthreads; i++) {
        s_->mergeTallies(*(sims[i]));
        s_->mergeEvents(*(sims[i]));
    }

    // report progress for the last time
    if (cb) {
        for(int i=0; i<nthreads; i++)
            thread_ion_count_[i] = sims[i]->thread_ion_count();
        ion_count_ = s_->ion_count();

        cb(*this,callback_user_data);
    }

    // CALC TIME/ion CLOCK_PROCESS_CPUTIME_ID
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end); // POSIX
    double t_secs = 1. * (t_end.tv_sec - t_start.tv_sec) / nthreads;
    t_secs += 1.e-9 * (t_end.tv_nsec - t_start.tv_nsec) / nthreads;
    ips_ = ion_count_/t_secs;
    end_time_ = std::time(nullptr);

    // copy back rng state from 1st clone
    s_->copyRngStateFrom(*(sims[0]));

    // delete simulation clones
    for(int i=0; i<nthreads; i++) {
        delete sims[i];
    }

    return 0;
}

int mcdriver::options::validate()
{
    // Driver
    if (Driver.max_no_ions <= 0)
        throw std::invalid_argument("Simulation.max_no_ions must be larger than 0.");

    if (Driver.threads <=0)
        throw std::invalid_argument("Simulation.threads must be 1 or larger.");

    // Simulation & Transport
    if (Simulation.scattering_calculation==mccore::InvalidScatteringOption)
        throw std::invalid_argument("Invalid Simulation.scattering_calculation");

    if (Transport.flight_path_type==mccore::InvalidPath)
        throw std::invalid_argument("Invalid Transport.flight_path_type");
    if (Transport.flight_path_type==mccore::Constant &&
        Transport.flight_path_const<=0.f)
        throw std::invalid_argument("Transport.flight_path_type is \"Constant\" but Transport.flight_path_const is negative.");

    if (Simulation.nrt_calculation==mccore::NRT_InvalidOption)
        throw std::invalid_argument("Invalid Simulation.nrt_calculation");

    if (Simulation.screening_type==Screening::None)
        throw std::invalid_argument("Invalid Simulation.screening_type");

    if (Simulation.straggling_model==StragglingModel::Invalid)
        throw std::invalid_argument("Invalid Simulation.straggling_model");

    // Ion source
    if (IonBeam.ion_distribution==ion_beam::InvalidIonDistribution)
        throw std::invalid_argument("Invalid IonBeam.ion_distribution");


    // Output
    const std::string& fname = Output.OutputFileBaseName;
    if (fname.empty())
        throw std::invalid_argument("Output.OutputFileBaseName is empty.");

    if (std::any_of(fname.begin(),
                    fname.end(),
                    [](unsigned char c){ return !(std::isalnum(c) || c=='_'); }))
    {
        std::string msg = "Output.OutputFileBaseName=\"";
        msg += fname;
        msg += "\" contains invalid characters. Valid chars=[0-9a-zA-Z_].";
        throw std::invalid_argument(msg);
    }

    // Target
    if (Target.materials.empty())
        throw std::invalid_argument("Target.materials is empty.");
    if (Target.regions.empty())
        throw std::invalid_argument("Target.regions is empty.");
    const ivector3& cell_count = Target.cell_count;
    if (std::any_of(cell_count.begin(),
                    cell_count.end(),
                    [](int c){ return c<=0; }))
    {
        std::stringstream msg;
        msg << "Target.cell_count=[";
        msg << cell_count;
        msg << "] contains zero or negative values.";
        throw std::invalid_argument(msg.str());
    }
    const vector3& cell_size = Target.cell_size;
    if (std::any_of(cell_size.begin(),
                    cell_size.end(),
                    [](float c){ return c<=0.f; }))
    {
        std::stringstream msg;
        msg << "Target.cell_size=[";
        msg << cell_size;
        msg << "] contains zero or negative values.";
        throw std::invalid_argument(msg.str());
    }

    // Check Material descriptors
    std::unordered_map<std::string, int> mmap; // map material_id->index
    for(int i=0; i<Target.materials.size(); ++i) {

        auto md = Target.materials[i];
        if (mmap.count(md.id)) {
            std::stringstream msg;
            msg << "Duplicate material id found: ";
            msg << md.id;
            throw std::invalid_argument(msg.str());
        }
        mmap[md.id] = i;

        if (md.density <= 0.f) {
            std::stringstream msg;
            msg << "Zero or negative density in material";
            msg << md.id;
            throw std::invalid_argument(msg.str());
        }
        CHECK_EMPTY_VEC(md,Z,md.id)
        CHECK_EMPTY_VEC(md,M,md.id)
        CHECK_EMPTY_VEC(md,X,md.id)
        CHECK_EMPTY_VEC(md,Ed,md.id)
        CHECK_EMPTY_VEC(md,El,md.id)
        CHECK_EMPTY_VEC(md,Es,md.id)
        CHECK_EMPTY_VEC(md,Er,md.id)

        int natoms = md.Z.size();
        if (std::any_of(md.Z.begin(),
                        md.Z.end(),
                        [](int z){ return (z < 1) || (z > elements::max_atomic_num);}))
        {
            throw std::invalid_argument("Invalid Z number in material" + md.id);
        }

        CHECK_VEC_SIZE(md,M,natoms,md.id)
        CHECK_VEC_SIZE(md,X,natoms,md.id)
        CHECK_VEC_SIZE(md,Ed,natoms,md.id)
        CHECK_VEC_SIZE(md,El,natoms,md.id)
        CHECK_VEC_SIZE(md,Es,natoms,md.id)
        CHECK_VEC_SIZE(md,Er,natoms,md.id)
        CHECK_VEC_ZEROorNEG(md,M,md.id)
        CHECK_VEC_ZEROorNEG(md,X,md.id)
        CHECK_VEC_ZEROorNEG(md,Ed,md.id)
        CHECK_VEC_ZEROorNEG(md,El,md.id)
        CHECK_VEC_ZEROorNEG(md,Es,md.id)
        CHECK_VEC_ZEROorNEG(md,Er,md.id)
    }

    // Check Region descriptors
    for(auto rd : Target.regions) {
        auto rname = rd.id;

        // check valid material
        if (!mmap.count(rd.material_id)) {
            std::stringstream msg;
            msg << "Region " << rname << " has invalid material_id: ";
            msg << rd.material_id;
            throw std::invalid_argument(msg.str());
        }

        // check max > min
        for (int i=0; i<3; i++) {
            if (rd.min[i] >= rd.max[i]) {
                static char axis[] = { 'x', 'y', 'z' };
                std::stringstream msg;
                msg << "Region " << rname << " has invalid axis limits: ";
                msg << axis[i] << "_min(" << rd.min[i] << ") >= ";
                msg << axis[i] << "_max(" << rd.max[i] << ")";
                throw std::invalid_argument(msg.str());
            }
        }

        // check that the region is within the simulation volume
        box3D rbox, // region box
            sbox; // simulation box
        rbox.min() = rd.min;
        rbox.max() = rd.max;
        sbox.min() = vector3(0,0,0);
        sbox.max() = vector3(Target.cell_count(0)*Target.cell_size(0),
                             Target.cell_count(1)*Target.cell_size(1),
                             Target.cell_count(2)*Target.cell_size(2));
        rbox = sbox.intersection(rbox);
        if (rbox.isEmpty()) {
            std::stringstream msg;
            msg << "Region " << rname << " does not intersect ";
            msg << "the simulation volume.";
            throw std::invalid_argument(msg.str());
        }
    }

    return 0;
}

mccore* mcdriver::options::createSimulation() const
{
    mccore* S = new mccore(Simulation, Transport);

    S->getSource().setParameters(IonBeam);

    target& T = S->getTarget();

    grid3D& G = T.grid();
    G.setX(Target.cell_count.x()*Target.cell_size.x(),
           Target.cell_count.x(), Target.periodic_bc.x());
    G.setY(Target.cell_count.y()*Target.cell_size.y(),
           Target.cell_count.y(), Target.periodic_bc.y());
    G.setZ(Target.cell_count.z()*Target.cell_size.z(),
           Target.cell_count.z(), Target.periodic_bc.z());

    for(auto md : Target.materials) T.addMaterial(md);
    for(auto rd : Target.regions) T.addRegion(rd);

    return S;
}
