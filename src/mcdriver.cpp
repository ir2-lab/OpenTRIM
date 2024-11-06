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
    opt.Simulation = s_->getParameters();
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

    int nthreads = par_.threads;
    if (nthreads < 1) nthreads = 1;

    // TIMING
    start_time_ = std::time(nullptr);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start);

    // create simulation clones
    std::vector< mccore* > sims(nthreads);
    for(int i=0; i<nthreads; i++) sims[i] = new mccore(*s_);

    // open streams
    for(int i=0; i<nthreads; i++) {
        if (out_opts_.store_pka)
            sims[i]->open_pka_stream(outFileName("pka",i).c_str());
        if (out_opts_.store_transmitted_ions)
            sims[i]->open_exit_stream(outFileName("exit",i).c_str());
    }
    if (out_opts_.store_pka) {
        s_->open_pka_stream(outFileName("pka").c_str());
        s_->close_pka_stream();
    }
    if (out_opts_.store_transmitted_ions) {
        s_->open_exit_stream(outFileName("exit").c_str());
        s_->close_exit_stream();
    }

    // if no seeds given, generate random seeds
    std::vector<unsigned int> myseeds = par_.seeds;
    if (myseeds.size() < nthreads) {
        myseeds.resize(nthreads);
        std::random_device rd;
        for(int i = 0; i<nthreads; i++)
            myseeds[i] = rd();

    }

    // set max ions and seed in each thread
    // reset abort flag
    for(int i=0; i<nthreads; i++) {
        sims[i]->setMaxIons(par_.max_no_ions);
        sims[i]->seed(myseeds[i]);
        sims[i]->clear_abort_flag();
    }

    // create & start worker threads
    std::vector< std::thread > thread_pool;
    for(int i=0; i<nthreads; i++)
        thread_pool.emplace_back(&mccore::run, sims[i]);


    // report progress if callback function is given
    if (cb) {
        thread_ion_count_.assign(nthreads,0);
        do  {

            for(int i=0; i<nthreads; i++)
                thread_ion_count_[i] = sims[i]->thread_ion_count();
            ion_count_ = s_->ion_count();

            // consolidate results
            {
                std::lock_guard< std::mutex > lock(*(s_->tally_mutex()));
                for(int i=0; i<nthreads; i++)
                    s_->merge(*(sims[i]));
            }

            cb(*this,callback_user_data);

            std::this_thread::sleep_for(std::chrono::milliseconds(msInterval));

        } while(ion_count_ < par_.max_no_ions && !(s_->abort_flag()));
    }

    // wait for threads to finish...
    for(int i=0; i<nthreads; i++) thread_pool[i].join();

    // consolidate results
    for(int i=0; i<nthreads; i++)
        s_->merge(*(sims[i]));

    // CALC TIME/ion CLOCK_PROCESS_CPUTIME_ID
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end); // POSIX
    double t_secs = 1. * (t_end.tv_sec - t_start.tv_sec) / nthreads;
    t_secs += 1.e-9 * (t_end.tv_nsec - t_start.tv_nsec) / nthreads;
    ips_ = s_->getTally().Nions()/t_secs;
    end_time_ = std::time(nullptr);


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

    if (!Driver.seeds.empty()) {
        if (Driver.seeds.size()!=Driver.threads) {
            std::stringstream ss;
            ss << "Driver.threads=" << Driver.threads << " while the # of "
               << "Driver.seeds is " << Driver.seeds.size() << "." << std::endl
               << "Either enter a # of seeds equal to the # of threads or no seeds at all.";
            throw std::invalid_argument(ss.str());
        }
    }

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
                    [](unsigned char c){ return !std::isalnum(c); }))
    {
        std::string msg = "Output.OutputFileBaseName=\"";
        msg += fname;
        msg += "\" contains non alphanumeric characters.";
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
    //S->setOutputOptions(Output);
    S->getSource().setParameters(IonBeam);

    target& T = S->getTarget();

    std::unordered_map<std::string, material*> materials_map;

    for(auto md : Target.materials) {
        material* m = T.addMaterial(md.id.c_str());
        materials_map[md.id] = m;
        m->setMassDensity(md.density);
        for(int i=0; i<md.Z.size(); i++)
            m->addAtom(
                {.Z=md.Z[i],.M=md.M[i],.Ed=md.Ed[i],
                 .El=md.El[i],.Es=md.Es[i],.Er=md.Er[i]},
                md.X[i]);
    }

    grid3D& G = T.grid();
    G.setX(Target.cell_count.x()*Target.cell_size.x(),
           Target.cell_count.x(), Target.periodic_bc.x());
    G.setY(Target.cell_count.y()*Target.cell_size.y(),
           Target.cell_count.y(), Target.periodic_bc.y());
    G.setZ(Target.cell_count.z()*Target.cell_size.z(),
           Target.cell_count.z(), Target.periodic_bc.z());

    for(auto rd : Target.regions) {
        box3D box;
        box.min() = rd.min;
        box.max() = rd.max;
        T.fill(box,materials_map[rd.material_id]);
    }

    return S;
}
