#include "mcdriverobj.h"

#include "mcdriver.h"
#include "ionsui.h"

#include <QMessageBox>
#include <QFile>
#include <QApplication>


// McDriverObj should not have a parent
// because it lives in a separate thread
McDriverObj::McDriverObj(IonsUI *iui)
    : QObject(),
    driver_(new mcdriver),
    ionsui_(iui),
    is_running_(false)
{
    mcdriver::options opt;
    std::stringstream os;
    opt.printJSON(os);
    jsonOptions = QJsonDocument::fromJson(os.str().c_str());

    connect(this, &McDriverObj::startSignal,
            this, &McDriverObj::start_,Qt::QueuedConnection);

}

McDriverObj::~McDriverObj()
{
    delete driver_;
}

void McDriverObj::mc_callback_(const mcdriver& d, void* p)
{
    McDriverObj* me = static_cast<McDriverObj*>(p);

    ArrayNDd& totals_ = me->totals_;
    me->driver_->getSim()->copyTallyTable(0,totals_);
    if (me->totals_[0]>0.)
        for(int i=1; i<totals_.size(); ++i)
            totals_[i] /= totals_[0];

    emit me->tallyUpdate();
}

bool McDriverObj::validateOptions(bool noMsgIfOK) const
{
    mcdriver::options opt;

    std::string s(jsonOptions.toJson().constData());
    std::stringstream ss(s, std::ios_base::in);

    int ret = opt.parseJSON(ss,false);

    if (ret) {
        QMessageBox::critical(ionsui_, "Options validation", "Internal error in JSON opts !!");
        return false;
    }

    bool isValid = true;
    QString msg;

    try {
        opt.validate();
    } catch (const std::invalid_argument& e) {
        isValid = false;
        msg = QString("Errors found:\n%1").arg(e.what());
    }


    if (!isValid)
        QMessageBox::warning(ionsui_, "Options validation", msg);

    if (isValid && !noMsgIfOK)
        QMessageBox::information(ionsui_, "Options validation","Options are OK!");

    return isValid;
}

McDriverObj::DriverStatus McDriverObj::status() const
{
    const mccore* sim = driver_->getSim();
    if (sim == nullptr) return mcReset;
    else return is_running_ ? mcRunning : mcIdle;
}

void McDriverObj::loadJson(const QString &path)
{
    reset();

    mcdriver::options opt;

    if (!path.isNull()) {
        // path should point to a valid config file
        // open file and read config, no checks!
        QFile f(path);
        f.open( QFile::ReadOnly );
        mcdriver::options opt;
        std::stringstream is(f.readAll().constData());
        bool validate = false;
        opt.parseJSON(is,validate);
    }

    std::stringstream os;
    opt.printJSON(os);
    jsonOptions = QJsonDocument::fromJson(os.str().c_str());

    ionsui_->optionsView->revert();
    ionsui_->runView->revert();
}

bool McDriverObj::start(bool b)
{
    if (is_running_) {
        if (!b) driver_->abort();
    } else if (b) {
        // get override options
        RunView* runView = ionsui_->runView;
        size_t max_no_ions = runView->max_ions();
        int nthreads = runView->nthreads();
        unsigned int seed = runView->seed();
        size_t updInterval = runView->updInterval();

        if (driver_->getSim() == nullptr) {
            if (!validateOptions(true)) return false;
            mcdriver::options opt;
            std::string s(jsonOptions.toJson().constData());
            std::stringstream ss(s, std::ios_base::in);
            opt.parseJSON(ss,false);
            opt.Driver.max_no_ions = max_no_ions;
            // opt.Driver.seeds = ToDo fix seed
            opt.Driver.threads = nthreads;
            opt.Output.storage_interval = updInterval;

            driver_->setOptions(opt);

            total_elapsed_ = 0;

            emit simulationCreated();

        } else {
            mcdriver::parameters par = driver_->driverOptions();
            par.max_no_ions = max_no_ions;
            par.threads = nthreads;
            driver_->setDriverOptions(par);

            mcdriver::output_options opts = driver_->outputOptions();
            opts.storage_interval = updInterval;
            driver_->setOutputOptions(opts);
        }

        emit startSignal();
    }
    return true;
}

void McDriverObj::start_()
{
    size_t updInterval = ionsui_->runView->updInterval();

    is_running_ = true;
    emit statusUpdate();

    init_run_data();

    int ret = -1;
    if (driver_->getSim())
        ret = driver_->exec(mc_callback_,updInterval,this);

    is_running_ = false;
    update_run_data();
    eta_ = 0;
    total_elapsed_ += elapsed_;
    elapsed_ = 0.;

    emit statusUpdate();
}

void McDriverObj::init_run_data()
{  
    tstart_ = my_clock_t::now();
    nstart_ = driver_->getSim()->ion_count();
    ncurr_ = nstart_;
    ntarget_ = driver_->driverOptions().max_no_ions;
    progress_ = int(1000.0*ncurr_/ntarget_);
    elapsed_ = 0.;
    eta_ = std::numeric_limits<double>::infinity();
    ips_ = 0.;
    totals_ = driver_->getSim()->getTallyTable(0);
    if (totals_[0]>0.)
        for(int i=1; i<totals_.size(); ++i) totals_[i] /= totals_[0];
}
void McDriverObj::update_run_data()
{
    time_point t = my_clock_t::now();
    ncurr_ = driver_->getSim()->ion_count();
    // floating-point duration: no duration_cast needed
    const std::chrono::duration<double> fp_sec = t - tstart_;
    elapsed_ = fp_sec.count();
    ips_ = (ncurr_ - nstart_)/elapsed_;
    eta_ = (ntarget_ - ncurr_)/ips_;
    progress_ = int(1000.0*ncurr_/ntarget_);
}

const tally &McDriverObj::getTally() const
{
    return driver_->getSim()->getTally();
}

const mccore *McDriverObj::getSim() const
{
    return driver_->getSim();
}

void McDriverObj::reset()
{
    if (status() == mcRunning) {
        start(false);
        while(status() == mcRunning)
            QCoreApplication::processEvents(QEventLoop::AllEvents,100);

    }
    if (status() == mcIdle) {
        driver_->reset();
        emit simulationDestroyed();
    }
}


