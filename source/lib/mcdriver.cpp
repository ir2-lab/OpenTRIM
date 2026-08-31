#include "mcdriver.h"
#include "periodic_table.h"

#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>

mcdriver::mcdriver(const mcconfig &cfg) : config_(cfg), s_(nullptr)
{
    config_.validate(false, nullptr);

    s_ = std::unique_ptr<mccore>(new mccore(cfg.Simulation, cfg.Transport));

    s_->getSource().setParameters(cfg.IonBeam);

    target &T = s_->getTarget();
    T.createGrid(cfg.Target.origin, cfg.Target.size, cfg.Target.cell_count, cfg.Target.periodic_bc);

    for (auto md : cfg.Target.materials)
        T.addMaterial(md);

    for (auto rd : cfg.Target.regions)
        T.addRegion(rd);

    for (int i = 0; i < int(cfg.UserTally.size()); ++i)
        s_->addUserTally(cfg.UserTally[i]);

    s_->init();
}

std::shared_ptr<mcdriver> mcdriver::create(const mcconfig &cfg, std::ostream *os)
{
    std::shared_ptr<mcdriver> D;

    try {
        D = std::shared_ptr<mcdriver>(new mcdriver(cfg));
    } catch (std::invalid_argument &e) {
        if (os)
            (*os) << e.what() << std::endl;
        return std::shared_ptr<mcdriver>();
    } catch (std::exception &e) {
        if (os)
            (*os) << e.what() << std::endl;
        return std::shared_ptr<mcdriver>();
    }

    return D;
}

mcdriver::~mcdriver()
{
    if (s_) {
        abort();
        wait();
    }
}

void mcdriver::abort()
{
    if (s_)
        s_->abort();
}

void mcdriver::wait()
{
    for (int i = 0; i < (int)thread_pool_.size(); ++i)
        thread_pool_[i].join();
}

bool mcdriver::install_event_handler(mccore::event_handler h, uint32_t mask, void *p, int thread_no)
{
    event_handlers_.push_back({ h, p, mask, thread_no });
    return true;
}

void mcdriver::clear_event_handlers()
{
    event_handlers_.clear();
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
        ev_mask |= pka_buffer::event_mask;
    if (config_.Output.store_exit_events)
        ev_mask |= exit_buffer::event_mask;
    if (config_.Output.store_damage_events)
        ev_mask |= damage_event_buffer::event_mask;

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
        // 1st ion id for this thread
        size_t id1 = n_start + i + 1;
        // ions for this thread
        size_t thread_n_ions = (n_run / nthreads) + (i < (n_run % nthreads) ? 1 : 0);
        sim_clones_[i]->arm(thread_n_ions, id1, nthreads);
    }

    // set event handlers
    for (auto eh : event_handlers_) {
        if (eh.thread_no < (int)nthreads)
            sim_clones_[eh.thread_no]->set_event_handler(eh.eh, eh.mask, eh.user_data);
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
            cb(this, callback_user_data);
        }

    } while ((s_->ion_count() < n_end) && !(s_->abort_flag()));

    // wait for threads to finish...
    for (size_t i = 0; i < nthreads; i++)
        thread_pool_[i].join();

    // if the actual total ion count is less than the expected
    // (due to the simulation being aborted by the user or due to
    // the time limit reached)
    // we have to check if we have all consequtive ion history ids
    if (s_->ion_count() < n_end) {

        // update the last ion count (n_end) and
        // the # of ions run in this exec()
        n_end = s_->ion_count();
        n_run = n_end - n_start;

        // get the last ion ID in each thread
        std::vector<size_t> ids(nthreads);
        for (size_t i = 0; i < nthreads; i++)
            ids[i] = sim_clones_[i]->thread_ion_count() ? sim_clones_[i]->next_ion_id() - nthreads
                                                        : size_t(-1);

        // get the max ID and the thread index where it occured
        auto max_it = std::max_element(ids.begin(), ids.end());
        int i = std::distance(ids.begin(), max_it);
        size_t maxId = *max_it;

        // if maxID > n_end => missing IDs
        if (maxId > n_end) {
            // how many
            size_t n_missing = maxId - n_end;
            // check all other threads (except the one with maxID) to find the missing
            // traverse the threads from i(maxID) backwards
            for (int k = 0; k < nthreads - 1; ++k) {
                // update the thread index
                i--;
                if (i < 0)
                    i += nthreads;
                // this should be the i-th thread maxID
                maxId--;
                // check if the last id is below that
                while ((ids[i] < maxId || ids[i] == size_t(-1)) && n_missing) {
                    if (ids[i] == size_t(-1)) {
                        ids[i] = n_start + i + 1;
                    } else
                        ids[i] += nthreads;
                    // simulate 1 missing ion
                    sim_clones_[i]->arm(1, ids[i], nthreads);
                    sim_clones_[i]->run();
                    n_missing--;
                }
                if (!n_missing)
                    break;
            }
            assert(!n_missing);
        } else {
            assert(maxId == n_end);
        }
    }

    // consolidate tallies
    for (size_t i = 0; i < nthreads; i++) {
        s_->mergeTallies(*(sim_clones_[i]));
    }
    // consolidate events, ordered per history id
    s_->mergeEvents(sim_clones_);

    // report progress for the last time
    if (cb) {
        cb(this, callback_user_data);
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

    // delete simulation clones, clear threads & clone pointers
    for (int i = 0; i < nthreads; i++) {
        delete sim_clones_[i];
    }
    thread_pool_.clear();
    sim_clones_.clear();

    return 0;
}

using std::endl;

#define CHECK_INVALID_ENUM(OptName, EnumName)          \
    if (int(OptName.EnumName) < 0) {                   \
        msg << "(/" << #OptName "/" #EnumName << ") "; \
        msg << " Invalid enum value." << endl;         \
        ret = false;                                   \
    }

#define CHECK_INVALID_ENUM2(OptName1, OptName2, EnumName)             \
    if (int(OptName1.OptName2.EnumName) < 0) {                        \
        msg << "(/" << #OptName1 "/" #OptName2 "/" #EnumName << ") "; \
        msg << " Invalid enum value." << endl;                        \
        ret = false;                                                  \
    }

int mcconfig::validate(bool AcceptIncomplete, std::ostream *os) const
{
    bool ret = true;
    std::ostringstream msg;

    // Simulation & Transport
    CHECK_INVALID_ENUM(Simulation, simulation_type)
    CHECK_INVALID_ENUM(Simulation, screening_type)
    CHECK_INVALID_ENUM(Simulation, electronic_stopping)
    CHECK_INVALID_ENUM(Simulation, electronic_straggling)
    CHECK_INVALID_ENUM(Simulation, nrt_calculation)
    CHECK_INVALID_ENUM(Transport, flight_path_type)

    if (Transport.flight_path_type == flight_path_calc::Constant
        && Transport.flight_path_const <= 0.f) {
        msg << "(/Transport/flight_path_const) Negative flight path constant with "
               "flight_path_type==Constant"
            << endl;
        ret = false;
    }

    // Ion source
    CHECK_INVALID_ENUM2(IonBeam, energy_distribution, type)
    CHECK_INVALID_ENUM2(IonBeam, spatial_distribution, type)
    CHECK_INVALID_ENUM2(IonBeam, spatial_distribution, geometry)
    CHECK_INVALID_ENUM2(IonBeam, angular_distribution, type)

    // Output
    const std::string &fname = Output.outfilename;
    if (fname.empty() && !AcceptIncomplete) {
        msg << "(/Output/outfilename) is empty. " << endl;
        ret = false;
    }

    if (!fname.empty() && std::any_of(fname.begin(), fname.end(), [](unsigned char c) {
            return !(std::isalnum(c) || c == '_');
        })) {
        msg << "(/Output/outfilename) \"";
        msg << fname;
        msg << "\" contains invalid characters. Valid chars=[0-9a-zA-Z_]." << endl;
        ret = false;
    }

    std::unordered_map<std::string, int> mmap; // map material_id->index
    bool target_ret = true;

    // Target
    if (!Target.materials.empty()) {
        // Check Material descriptors
        for (int i = 0; i < Target.materials.size(); ++i) {

            auto md = Target.materials[i];
            if (md.id.empty()) {
                msg << "(/Target/materials/" << i << "/id) Empty material id." << endl;
                target_ret = false;
            }
            if (mmap.count(md.id)) {
                msg << "(/Target/materials) Duplicate material id found: ";
                msg << '"' << md.id << '"' << endl;
                target_ret = false;
            }
            mmap[md.id] = i;

            if (md.density == 0.f) {
                msg << "(/Target/materials/" << i << "/density) Undefined or zero density" << endl;
                target_ret = false;
            }
            if (md.density < 0.f) {
                msg << "(/Target/materials/" << i << "/density) Negative density" << endl;
                target_ret = false;
            }
            if (md.composition.empty()) {
                msg << "(/Target/materials/" << i << "/composition) is empty" << endl;
                target_ret = false;
            }

            std::set<int> atset;
            for (int j = 0; j < int(md.composition.size()); ++j) {
                const atom::parameters &at = md.composition[j];
                int Z = at.element.atomic_number;
                if (atset.count(Z)) {
                    msg << "(/Target/materials/" << i << "/composition/" << j << "/element) ";
                    ret = false;
                    msg << "Duplicate element " << periodic_table::at(Z).symbol << "(Z=" << Z << ")"
                        << endl;
                    target_ret = false;
                }
                atset.insert(Z);
            }
        }
    }

    if (!Target.regions.empty()) {

        // Check Region descriptors
        for (int i = 0; i < int(Target.regions.size()); ++i) {

            auto rd = Target.regions[i];
            auto rname = rd.id;

            // check valid material
            if (!mmap.count(rd.material_id)) {
                msg << "(/Target/regions/" << i << "/material_id) Undefined ";
                msg << "material_id: " << rd.material_id << endl;
                target_ret = false;
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
                msg << "(/Target/regions/" << i << ") Region ";
                msg << rname << " does not intersect with ";
                msg << "the simulation volume." << endl;
                target_ret = false;
            }
        }
    }
    ret = ret && target_ret;

    // --- UserTally ---
    std::unordered_map<std::string, int> utmap; // map UserTally id->index
    for (int i = 0; i < int(UserTally.size()); ++i) {

        auto ut = UserTally[i];
        if (ut.id.empty()) {
            msg << "(/UserTally/" << i << "/id) Empty user tally id." << endl;
            ret = false;
        }
        if (utmap.count(ut.id)) {
            msg << "(/UserTally/" << i << "/id) Duplicate user tally id found: ";
            msg << '"' << ut.id << '"' << endl;
            ret = false;
        }
        utmap[ut.id] = i;

        // coord sys
        vector3 nz = ut.coordinate_system.zaxis.normalized();
        vector3 xzn = ut.coordinate_system.xzvector.normalized();
        if (std::abs(std::abs(nz.dot(xzn)) - 1) < 10 * std::numeric_limits<float>::epsilon()) {
            msg << "(/UserTally/" << i << "/coordinate_system) zaxis is nearly parallel to xzvector"
                << endl;
            ret = false;
        }

        int nbins = 0;
        auto ff = [&msg](const std::vector<float> &b, const std::string &path, int &count) -> bool {
            int n = b.size();
            if (n == 0)
                return true; // empty is ok
            if (n == 1) {
                msg << "(" << path << ") Has size=1. At least 2 bin edges are required" << endl;
                return false;
            }
            bool asc = true;
            for (int i = 0; i < n - 1; ++i)
                if (b[i] >= b[i + 1]) {
                    asc = false;
                    break;
                }

            if (!asc) {
                msg << "(" << path << ") is not strictly increasing." << endl;
                return false;
            }

            count++;
            return true;
        };
        std::string path = "/UserTally/";
        path += std::to_string(i);
        path += "/bins/";
        ret = ret && ff(ut.bins.x, path + "x", nbins);
        ret = ret && ff(ut.bins.y, path + "y", nbins);
        ret = ret && ff(ut.bins.z, path + "z", nbins);
        ret = ret && ff(ut.bins.r, path + "r", nbins);
        ret = ret && ff(ut.bins.rho, path + "rho", nbins);
        ret = ret && ff(ut.bins.cosTheta, path + "cosTheta", nbins);
        ret = ret && ff(ut.bins.nx, path + "nx", nbins);
        ret = ret && ff(ut.bins.ny, path + "ny", nbins);
        ret = ret && ff(ut.bins.nz, path + "nz", nbins);
        ret = ret && ff(ut.bins.E, path + "E", nbins);
        ret = ret && ff(ut.bins.Tdam, path + "Tdam", nbins);
        ret = ret && ff(ut.bins.V, path + "V", nbins);
        ret = ret && ff(ut.bins.atom_id, path + "atom_id", nbins);
        ret = ret && ff(ut.bins.recoil_id, path + "recoil_id", nbins);

        if (nbins == 0) {
            msg << "(/UserTally/" << i << ") No valid bins found." << endl;
            ret = false;
        }
    }

    if (ret)
        return 0;

    if (os) {
        *os << msg.str();
    } else {
        throw std::invalid_argument(msg.str());
    }
    return -1;
}
