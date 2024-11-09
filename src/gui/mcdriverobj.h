#ifndef MCDRIVEROBJ_H
#define MCDRIVEROBJ_H

#include <QObject>
#include <QJsonDocument>

#include <chrono>

#include "arrays.h"
#include "mccore.h"

class IonsUI;
class mcdriver;
class mccore;
class tally;

class McDriverObj : public QObject
{
    Q_OBJECT

public:

    QJsonDocument jsonOptions;

    explicit McDriverObj(IonsUI *iui);
    virtual ~McDriverObj() override;

    bool validateOptions(bool noMsgIfOK = false) const;

    enum DriverStatus {
        mcReset,
        mcIdle,
        mcRunning
    };

    DriverStatus status() const;

    void loadJson(const QString& path);

    bool start(bool b);
    void reset();

    void init_run_data();
    void update_run_data();
    int progress() const { return progress_; }
    double elapsed() const { return total_elapsed_ + elapsed_; }
    double ips() const { return ips_; }
    double eta() const { return eta_; }
    size_t nions() const { return ncurr_; }
    const ArrayNDd& totals() const { return totals_; }

    const tally& getTally() const;
    const mccore* getSim() const;


private slots:
    void start_();

signals:
    void simulationCreated();
    void simulationDestroyed();
    void startSignal();

    // these are sent from within the worker/simulation thread
    // must be connected with Qt::QueuedConn.
    void statusUpdate();
    void tallyUpdate();

private:
    mcdriver* driver_;
    IonsUI* ionsui_;
    std::atomic_bool is_running_;

    // state
    typedef std::chrono::high_resolution_clock my_clock_t;
    typedef my_clock_t::time_point time_point;

    time_point tstart_; // time when sim started
    size_t nstart_; // # of ions when starting
    size_t ncurr_; // # of ions current
    size_t ntarget_; // max # of ions
    int progress_; // 0..100 in percent
    double elapsed_;
    double total_elapsed_;
    double eta_; // s
    double ips_; // ions per second
    ArrayNDd totals_;

    static void mc_callback_(const mcdriver& d, void* p);
};

#endif // MCDRIVEROBJ_H
