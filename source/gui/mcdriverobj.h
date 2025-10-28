#ifndef MCDRIVEROBJ_H
#define MCDRIVEROBJ_H

#include <chrono>

#include <QObject>
#include <QTimer>

#include "mcdriver.h"

/*
 * QObject encalpsulating the OpenTRIM simulation driver
 *
 * The object "lives" in a separate Qt worker thread with its own message queue
 * (see MainUI constructor)
 *
 * The simulation is run from this worker thread.
 *
 * Note that a simulation spawns its own separate threads. The worker thread
 * waits for the simulation to end
 *
 */
class McDriverObj : public QObject
{
    Q_OBJECT

public:
    // helper class for getting real-time info
    // for the running simulation
    class running_sim_info
    {

        // timing
        typedef std::chrono::high_resolution_clock hr_clock_t;
        typedef hr_clock_t::time_point time_point;
        // sim data
        time_point tstart_; // time when sim started
        size_t nstart_; // # of ions when started
        size_t ncurr_; // # of ions current
        size_t ntarget_; // max # of ions
        int progress_; // 0..1000
        double elapsed_;
        double total_elapsed_;
        double etc_; // estimated time to completion (s)
        double ips_; // ions per second

        friend class McDriverObj;

        // clear = zero out everything
        void clear();
        // init : called before simulation starts
        void init(const McDriverObj &D);
        // update : called during simulation run
        void update(const McDriverObj &D);

    public:
        int progress() const { return progress_; }
        double elapsed() const { return total_elapsed_ + elapsed_; }
        double ips() const { return ips_; }
        double etc() const { return etc_; }
        size_t nions() const { return ncurr_; }
    };

    explicit McDriverObj();
    virtual ~McDriverObj() override;

    // get/set the simulation mcconfig
    const mcconfig &options() const;
    void setOptions(const mcconfig &opt, bool initFromFile);
    // return options as json
    std::string json() const;

    const mcdriver *get_mcdriver() const { return driver_; }

    // get/set modified flag
    // modified = sim options or data have changed
    bool isModified() const { return modified_; }
    void setModified(bool b);
    // get/set template flag
    // template = the sim is a template or example
    bool isTemplate() const { return template_; }
    void setTemplate(bool b);

    // get/set file name or full path
    QString fileName() const { return fileName_; }
    QString filePath() const { return filePath_; }
    void setFileName(const QString &path);
    void setTemplateName(const QString &name);

    // get sim title
    QString title() const;

    // validate sim options
    // error msgs are optionally returned in msg pointer
    bool validateOptions(QString *msg = nullptr) const;

    enum DriverStatus {
        mcReset = 0, // reset state = mccore object does not exist,
                     //               options can be changed
        mcIdle = 1, // mccore created, not running
        mcRunning = 2, // simulation running
        mcMax = 3
    };

    // get driver status
    DriverStatus status() const { return (DriverStatus) int(status_); };

    // load a json example or the default if path = QString()
    void loadJsonTemplate(const QString &path = QString());
    // load from a file on disk
    void loadJsonFile(const QString &path);
    bool loadH5File(const QString &path);
    QString ioErrorMsg() const;

    void saveJson(const QString &fname);
    bool saveH5(const QString &fname);

    void start(bool b);
    void reset();

    // get real time simulation info
    const running_sim_info &sim_info()
    {
        info_.update(*this);
        return info_;
    }
    const ArrayNDd &totals() const { return totals_; }
    const ArrayNDd &dtotals() const { return dtotals_; }

    const tally &getTally() const;
    const mccore *getSim() const;

    // get run parameters
    size_t maxIons() const { return max_ions_; }
    int nThreads() const { return nThreads_; }
    int seed() const { return seed_; }
    int updInterval() const { return updInterval_; }

public slots:

    // set run parameters
    void setMaxIons(int n) { max_ions_ = n; };
    void setNThreads(int n) { nThreads_ = n; };
    void setSeed(int n) { seed_ = n; }
    void setUpdInterval(int n) { updInterval_ = n; }

private slots:

    // internal slots to pass commands to
    // worker thread

    // start the simulation
    void start_();
    // load HDF5 file
    void onLoadH5_(const QString &path);
    // save data to HDF5
    void onSaveH5_();

signals:
    void modificationChanged(bool b);
    void configChanged();
    void contentsChanged();
    void fileNameChanged();
    void simulationCreated();
    void simulationDestroyed();
    void startSignal();
    void simulationStarted(bool b);
    void loadH5_(const QString &path);
    void saveH5_();

    // these are sent from within the worker/simulation thread
    // must be connected with Qt::QueuedConn.
    void statusChanged();
    void tallyUpdate();

private:
    mcdriver *driver_;
    mcdriver *test_driver_;
    mcconfig options_;
    bool modified_;
    bool template_;
    QString fileName_;
    QString filePath_;
    std::atomic_bool is_running_;
    std::atomic_int status_;
    std::atomic_bool io_op_active_;
    int io_ret_;
    std::string io_err_;

    size_t max_ions_;
    int nThreads_;
    int seed_;
    int updInterval_;

    // run info
    running_sim_info info_;
    friend class running_sim_info;

    // tally totals - to be updated in regular intervals
    void update_tally_totals_();
    ArrayNDd totals_, dtotals_;

    void setStatus(DriverStatus s);

    static void mc_callback_(const mcdriver &d, void *p);
};

#endif // MCDRIVEROBJ_H
