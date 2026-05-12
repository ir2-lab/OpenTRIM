#include "mcdriver.h"
#include "periodic_table.h"

#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>

#define CHECK_INVALID_ENUM(OptName, EnumName) \
    if (int(OptName.EnumName) < 0)            \
        throw std::invalid_argument("Invalid enum value in " #OptName "." #EnumName);

#define CHECK_ZEROorNEG(OptName, Field) \
    if (int(OptName.Field) < 0)         \
        throw std::invalid_argument("Zero or negative " #OptName "." #Field);

#define CHECK_EMPTY_VEC(MatDesc, V, MatName) \
    if (MatDesc.V.size() == 0)               \
        throw std::invalid_argument("Empty " #V " in " + MatName);

#define CHECK_VEC_SIZE(MatDesc, V, N, MatName)                          \
    if (MatDesc.V.size() != N)                                          \
        throw std::invalid_argument("In material descriptor " + MatName \
                                    + " the # of values in " #V " is not equal to those of Z.");

#define ANY_ZEROorNEG(V) std::any_of(V.begin(), V.end(), [](float c) { return c <= 0.f; })

#define CHECK_VEC_ZEROorNEG(MatDesc, V, MatName)                        \
    if (ANY_ZEROorNEG(MatDesc.V))                                       \
        throw std::invalid_argument("In material descriptor " + MatName \
                                    + " " #V " has zero or negative values.");

#define CHECK_ZERO_ATOMPAR(AtomPar, V, MatName)                                            \
    if (AtomPar.V == 0.f)                                                                  \
        throw std::invalid_argument("Undefined or zero " #V " in element "                 \
                                    + AtomPar.element.symbol + " of material \"" + MatName \
                                    + "\"");

mcdriver::mcdriver() : s_(nullptr) { }

mcdriver::~mcdriver()
{
    reset();
}

std::string mcdriver::outFileName() const
{
    std::string s(config_.Output.outfilename);
    s += ".h5";
    return s;
}

void mcdriver::abort()
{
    if (s_)
        s_->abort();
}

void mcdriver::wait()
{
    for (int i = 0; i < thread_pool_.size(); ++i)
        thread_pool_[i].join();
}

void mcdriver::reset()
{
    if (s_) {
        abort();
        wait();
        delete s_;
        s_ = nullptr;
        run_history_.clear();
    }
}

void mcdriver::init(const mcconfig &config)
{
    reset();
    config_ = config;
    s_ = config_.createSimulation();
    s_->init();
}

double elapsed_sec(const timespec &t0, const timespec &t1)
{
    double d = 1. * (t1.tv_sec - t0.tv_sec);
    d += 1.e-9 * (t1.tv_nsec - t0.tv_nsec);
    return d;
}

int mcdriver::exec(progress_callback cb, size_t msInterval, void *callback_user_data)
{
    using namespace std::chrono_literals;
    static const size_t msTick = 100;
    size_t nTickPerInterval = std::max(msInterval / msTick, size_t(1));

    // wall clock time
    std::time_t start_time_, end_time_;
    // cpu time
    struct timespec t_start, t_end;

    size_t nthreads = config_.Run.threads;
    if (nthreads < 1) {
        nthreads = std::thread::hardware_concurrency();
        if (nthreads <= 3)
            nthreads = 1;
        else
            nthreads >>= 1; // use half the available threads
    }

    // TIMING
    start_time_ = std::time(nullptr);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start);

    // ion counts
    size_t n_start = s_->ion_count();
    size_t n_end = config_.Run.max_no_ions;
    if (n_end <= n_start)
        return -1;

    // ions to run
    size_t n_run = n_end - n_start;

    // check cpu time limit
    double tlim = std::numeric_limits<double>::max();
    if (config_.Run.max_cpu_time) {
        tlim = config_.Run.max_cpu_time;
        for (auto &rd : run_history_)
            tlim -= rd.cpu_time_s;
    }

    // If ion_count == 0, i.e. simulation starts, seed the rng
    if (s_->ion_count() == 0)
        s_->seed(config_.Run.seed);

    // create simulation clones
    sim_clones_.resize(nthreads);
    for (size_t i = 0; i < nthreads; i++)
        sim_clones_[i] = new mccore(*s_);

    // jump the rng's of clones (except the 1st one)
    for (size_t i = 1; i < nthreads; i++) {
        for (size_t j = 0; j < i; ++j)
            sim_clones_[i]->rngJump();
    }

    // init event streams
    uint32_t ev_mask{ 0 };
    if (config_.Output.store_pka_events)
        ev_mask |= static_cast<uint32_t>(Event::CascadeComplete);
    if (config_.Output.store_exit_events)
        ev_mask |= static_cast<uint32_t>(Event::IonExit);
    if (config_.Output.store_damage_events)
        ev_mask |= static_cast<uint32_t>(Event::Vacancy);

    // open clone streams
    for (size_t i = 0; i < nthreads; i++)
        sim_clones_[i]->init_streams(ev_mask);

    // If ion_count == 0, i.e. simulation starts,
    // open also the main simulation streams
    if (s_->ion_count() == 0)
        s_->init_streams(ev_mask);

    // arm the clones
    // each clone runs N/nthread ions +1 if i < N % nthread
    for (size_t i = 0; i < nthreads; i++) {
        size_t id0 = s_->ion_count() + i;
        size_t n_thread = (n_run / nthreads) + (i < (n_run % nthreads) ? 1 : 0);
        sim_clones_[i]->arm(n_thread, id0, nthreads);
    }

    // create & start worker threads
    for (size_t i = 0; i < nthreads; i++)
        thread_pool_.emplace_back(&mccore::run, sim_clones_[i]);

    // waiting loop
    do {

        // wait time in msTick intervals
        // always checking if simulation is finished or aborted
        size_t iTick = 0;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(msTick));
            iTick++;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
            if (elapsed_sec(t_start, t_end) >= tlim)
                abort();
        } while ((iTick < nTickPerInterval) && (s_->ion_count() < n_end) && !(s_->abort_flag()));

        // report progress if callback function is given
        if (cb) {
            // consolidate results
            for (size_t i = 0; i < nthreads; i++)
                s_->mergeTallies(*(sim_clones_[i]));
            // callback
            cb(*this, callback_user_data);
        }

    } while ((s_->ion_count() < n_end) && !(s_->abort_flag()));

    // wait for threads to finish...
    for (size_t i = 0; i < nthreads; i++)
        thread_pool_[i].join();

    // consolidate tallies
    for (size_t i = 0; i < nthreads; i++) {
        s_->mergeTallies(*(sim_clones_[i]));
    }
    // consolidate events, ordered per history id
    s_->mergeEvents(sim_clones_);

    // report progress for the last time
    if (cb) {
        cb(*this, callback_user_data);
    }

    // mark cpu time and world clock time
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end); // POSIX
    end_time_ = std::time(nullptr);

    // save run info
    run_data rd;
    rd.cpu_time_s = elapsed_sec(t_start, t_end);
    rd.run_ion_count = s_->ion_count() - n_start;
    rd.total_ion_count = s_->ion_count();
    rd.ions_per_cpu_s = rd.run_ion_count / rd.cpu_time_s;
    rd.nthreads = nthreads;
    // store ISO 8601 timestamps %Y-%m-%dT%H:%M:%SZ
    {
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&start_time_), "%Y-%m-%dT%H:%M:%SZ");
        rd.start_time = ss.str();
    }
    {
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&end_time_), "%Y-%m-%dT%H:%M:%SZ");
        rd.end_time = ss.str();
    }
    run_history_.push_back(rd);

    // copy back rng state from 1st clone
    s_->setRngState(sim_clones_[0]->rngState());

    // delete simulation clones
    for (int i = 0; i < nthreads; i++) {
        delete sim_clones_[i];
    }

    // clear threads & clone pointers
    thread_pool_.clear();
    sim_clones_.clear();

    return 0;
}

int mcconfig::validate(bool AcceptIncomplete)
{

    // Simulation & Transport
    CHECK_INVALID_ENUM(Simulation, simulation_type)
    CHECK_INVALID_ENUM(Simulation, screening_type)
    CHECK_INVALID_ENUM(Simulation, electronic_stopping)
    CHECK_INVALID_ENUM(Simulation, electronic_straggling)
    CHECK_INVALID_ENUM(Simulation, nrt_calculation)
    CHECK_INVALID_ENUM(Transport, flight_path_type)

    if (Transport.flight_path_type == flight_path_calc::Constant
        && Transport.flight_path_const <= 0.f)
        throw std::invalid_argument("Transport.flight_path_type is \"Constant\" but "
                                    "Transport.flight_path_const is negative.");

    // Ion source
    CHECK_INVALID_ENUM(IonBeam.energy_distribution, type)
    CHECK_INVALID_ENUM(IonBeam.spatial_distribution, type)
    CHECK_INVALID_ENUM(IonBeam.spatial_distribution, geometry)
    CHECK_INVALID_ENUM(IonBeam.angular_distribution, type)

    // Output
    const std::string &fname = Output.outfilename;
    if (fname.empty() && !AcceptIncomplete)
        throw std::invalid_argument("Output.outfilename is empty.");

    if (!fname.empty() && std::any_of(fname.begin(), fname.end(), [](unsigned char c) {
            return !(std::isalnum(c) || c == '_');
        })) {
        std::string msg = "Output.outfilename=\"";
        msg += fname;
        msg += "\" contains invalid characters. Valid chars=[0-9a-zA-Z_].";
        throw std::invalid_argument(msg);
    }

    std::unordered_map<std::string, int> mmap; // map material_id->index

    // Target
    if (!Target.materials.empty()) {
        // Check Material descriptors
        for (int i = 0; i < Target.materials.size(); ++i) {

            auto md = Target.materials[i];
            if (md.id.empty()) {
                std::stringstream msg;
                msg << "Empty material id in material No.";
                msg << i + 1;
                throw std::invalid_argument(msg.str());
            }
            if (mmap.count(md.id)) {
                std::stringstream msg;
                msg << "Duplicate material id found: ";
                msg << '"' << md.id << '"';
                throw std::invalid_argument(msg.str());
            }
            mmap[md.id] = i;

            if (md.density == 0.f) {
                std::stringstream msg;
                msg << "Undefined or zero density in material ";
                msg << '"' << md.id << '"';
                throw std::invalid_argument(msg.str());
            }
            if (md.density < 0.f) {
                std::stringstream msg;
                msg << "Negative density in material ";
                msg << '"' << md.id << '"';
                throw std::invalid_argument(msg.str());
            }

            if (md.composition.empty()) {
                std::stringstream msg;
                msg << "Undefined composition in material ";
                msg << '"' << md.id << '"';
                throw std::invalid_argument(msg.str());
            }

            std::set<int> atset;
            for (const atom::parameters &at : md.composition) {

                int Z = at.element.atomic_number;
                if (atset.count(Z)) {
                    std::stringstream msg;
                    msg << "Duplicate element " << periodic_table::at(Z).symbol << "(Z=" << Z
                        << ")";
                    msg << " in definition of material " << '"' << md.id << '"';
                    throw std::invalid_argument(msg.str());
                }
                atset.insert(Z);
            }
        }
    }

    if (Target.regions.empty()) {
        const ivector3 &cell_count = Target.cell_count;
        if (std::any_of(cell_count.begin(), cell_count.end(), [](int c) { return c <= 0; })) {
            std::stringstream msg;
            msg << "Target.cell_count=[";
            msg << cell_count;
            msg << "] contains zero or negative values.";
            throw std::invalid_argument(msg.str());
        }

        const vector3 &size = Target.size;
        if (std::any_of(size.begin(), size.end(), [](float c) { return c <= 0.f; })) {
            std::stringstream msg;
            msg << "Target.size=[";
            msg << size;
            msg << "] contains zero or negative values.";
            throw std::invalid_argument(msg.str());
        }

        // Check Region descriptors
        for (auto rd : Target.regions) {
            auto rname = rd.id;

            // check valid material
            if (!mmap.count(rd.material_id)) {
                std::stringstream msg;
                msg << "Region " << rname << " has invalid material_id: ";
                msg << rd.material_id;
                throw std::invalid_argument(msg.str());
            }

            const vector3 &size = rd.size;
            if (std::any_of(size.begin(), size.end(), [](float c) { return c <= 0.f; })) {
                std::stringstream msg;
                msg << "Region " << rname << ".size=[";
                msg << size;
                msg << "] contains zero or negative values.";
                throw std::invalid_argument(msg.str());
            }

            // check that the region is within the simulation volume
            box3D rbox, // region box
                    sbox; // simulation box
            rbox.min() = rd.origin;
            rbox.max() = rd.origin + rd.size;
            sbox.min() = Target.origin;
            sbox.max() = Target.origin + Target.size;
            rbox = sbox.intersection(rbox);
            if (rbox.isEmpty()) {
                std::stringstream msg;
                msg << "Region " << rname << " does not intersect ";
                msg << "the simulation volume.";
                throw std::invalid_argument(msg.str());
            }
        }
    }

    return 0;
}

mccore *mcconfig::createSimulation() const
{
    mccore *S = new mccore(Simulation, Transport);

    S->getSource().setParameters(IonBeam);

    target &T = S->getTarget();
    T.createGrid(Target.size, Target.cell_count, Target.periodic_bc);

    for (auto md : Target.materials)
        T.addMaterial(md);

    for (auto rd : Target.regions)
        T.addRegion(rd);

    for (int i = 0; i < UserTally.size(); ++i)
        S->addUserTally(UserTally[i]);

    return S;
}
