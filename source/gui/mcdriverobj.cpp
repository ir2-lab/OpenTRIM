#include "mcdriverobj.h"

#include "mcdriver.h"

#include <fstream>

#include <QFile>
#include <QApplication>
#include <QFileInfo>
#include <QProgressDialog>
#include <QElapsedTimer>


// McDriverObj should not have a parent
// because it lives in a separate thread
McDriverObj::McDriverObj()
    : QObject(),
    driver_(new mcdriver),
    modified_(false),
    is_running_(false),
    status_(0),
    max_ions_(100),
    nThreads_(1),
    seed_(123456789),
    updInterval_(1000)
{
    // internal start signal connection
    connect(this, &McDriverObj::startSignal,
            this, &McDriverObj::start_,Qt::QueuedConnection);

    // internal io operations signal connection
    connect(this, &McDriverObj::loadH5_,
            this, &McDriverObj::onLoadH5_,Qt::QueuedConnection);
    connect(this, &McDriverObj::saveH5_,
            this, &McDriverObj::onSaveH5_,Qt::QueuedConnection);
}

McDriverObj::~McDriverObj()
{
    delete driver_;
}

const mcdriver::options& McDriverObj::options() const
{
    return options_;
}

void McDriverObj::setOptions(const mcdriver::options &opt, bool initFromFile)
{
    options_ = opt;
    if (initFromFile) {
        max_ions_ = opt.Driver.max_no_ions;
        nThreads_ = opt.Driver.threads;
        seed_ = opt.Driver.seed;
        updInterval_ = opt.Output.storage_interval;
    }
    emit configChanged();
    setModified(true);
}

std::string McDriverObj::json() const
{
    mcdriver::options opt(options_);
    opt.Driver.max_no_ions = max_ions_;
    opt.Driver.seed = seed_;
    opt.Driver.threads = nThreads_;
    opt.Output.storage_interval = updInterval_;
    return opt.toJSON();
}

void McDriverObj::setModified(bool b)
{
    if (modified_ != b) {
        modified_ = b;
        emit modificationChanged(b);
    }
}

void McDriverObj::setTemplate(bool b)
{
    if (template_ != b) {
        template_ = b;
        emit modificationChanged(b);
    }
}

void McDriverObj::setFileName(const QString &path)
{
    QFileInfo finfo(path);
    QString baseName = finfo.baseName();
    bool updateName = filePath_.isEmpty();
    if (!updateName)
        updateName = finfo.absoluteFilePath()!=filePath_;
    if (!updateName)
        updateName = baseName != fileName();

    if (updateName)
    {
        fileName_ = baseName;
        filePath_ = finfo.absoluteFilePath();
        emit fileNameChanged();
    }
}

void McDriverObj::setTemplateName(const QString &name)
{
    fileName_ = name;
    filePath_ = "";
    emit fileNameChanged();
}

QString McDriverObj::title() const
{
    return QString::fromStdString(options_.Output.title);
}

void McDriverObj::mc_callback_(const mcdriver& d, void* p)
{
    McDriverObj* me = static_cast<McDriverObj*>(p);

    me->update_tally_totals_();

}

void McDriverObj::update_tally_totals_()
{
    totals_ = driver_->getSim()->getTallyTable(0);
    dtotals_ = driver_->getSim()->getTallyTableVar(0);
    if (totals_[0]>1.) {
        double f = 1./totals_[0];
        double f1 = 1./(totals_[0]-1.);
        for(int i=1; i<totals_.size(); ++i) {
            totals_[i] *= f;
            // error in the mean
            // S[i] = std::sqrt((dA[i]/N-M[i]*M[i])/(N-1));
            dtotals_[i] = std::sqrt((dtotals_[i]*f-totals_[i]*totals_[i])*f1);
        }
    }

    emit tallyUpdate();
}

bool McDriverObj::validateOptions(QString* msg) const
{
    mcdriver::options opt(options_);

    opt.Driver.max_no_ions = max_ions_;
    opt.Driver.seed = seed_;
    opt.Driver.threads = nThreads_;
    opt.Output.storage_interval = updInterval_;

    bool isValid = true;

    try {
        opt.validate();
    } catch (const std::invalid_argument& e) {
        isValid = false;
        if (msg) *msg = QString("Errors found:\n%1").arg(e.what());
    }

    return isValid;
}

void McDriverObj::loadJsonTemplate(const QString &path)
{
    reset();

    mcdriver::options opt;

    if (!path.isNull()) {
        // path should point to an example in resources.
        // open file and read config, no checks!
        QFile f(path);
        f.open( QFile::ReadOnly );
        std::stringstream is(f.readAll().constData());
        bool validate = false;
        opt.parseJSON(is,validate);
    }
    setOptions(opt, true);

    // In any case, this is called Untitled
    setTemplateName("Untitled");
    setTemplate(true);
    setModified(false);

    emit configChanged();
    emit contentsChanged();
    emit modificationChanged(false);
}

void McDriverObj::loadJsonFile(const QString &path)
{
    reset();

    mcdriver::options opt;

    // path should point to a valid config file.
    // open file and read config, no checks!
    QFile f(path);
    f.open( QFile::ReadOnly );
    std::stringstream is(f.readAll().constData());
    bool validate = false;
    opt.parseJSON(is,validate);

    setOptions(opt,true);

    setFileName(path);
    setTemplate(false);
    setModified(false);

    emit configChanged();
    emit contentsChanged();

    setStatus(mcReset);
}

bool McDriverObj::loadH5File(const QString &path)
{
    // Try to load the file
    QProgressDialog dlg("Loading HDF5. Please wait ...",QString(),0,110);
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setMinimumDuration( 0 );

    io_op_active_ = true;
    emit loadH5_(path);

    // TODO
    // Get file i/o progress indication from mcdriver
    int k = 0;
    QElapsedTimer tmr;
    tmr.start();
    while(io_op_active_) {
        dlg.setValue(k);
        if (tmr.hasExpired(250)) {
            k += 10;
            if (k>90) k=0;
            tmr.restart();
        }
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    if (io_ret_!=0) return false;

    reset();

    delete driver_;
    driver_ = test_driver_;
    test_driver_ = nullptr;

    mcdriver::options opt;
    driver_->getOptions(opt);
    setOptions(opt,true);

    setFileName(path);
    setTemplate(false);
    setModified(false);

    info_.init(*this);

    emit configChanged();
    emit simulationCreated();
    emit contentsChanged();

    setStatus(mcIdle);

    return true;
}

QString McDriverObj::ioErrorMsg() const
{
    return QString::fromStdString(io_err_);
}

void McDriverObj::saveJson(const QString &fname)
{
    setFileName(fname);

    // get from our local json
    mcdriver::options opt(options_);
    // if there is an active sim
    if (driver_->getSim())
        driver_->getOptions(opt); // get options from mccore object
    else { // get my current driver options
        opt.Driver.max_no_ions = max_ions_;
        opt.Driver.seed = seed_;
        opt.Driver.threads = nThreads_;
        opt.Output.storage_interval = updInterval_;
    }

    opt.Output.outfilename = fileName().toStdString();

    std::ofstream os(fname.toLatin1().constData());   
    opt.printJSON(os);

    setTemplate(false);
    setModified(false);
}

bool McDriverObj::saveH5(const QString &fname)
{
    if (!driver_->getSim()) return false;

    setFileName(fname);

    // Try to save the file
    QProgressDialog dlg("Saving HDF5. Please wait ...",QString(),0,110);
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setMinimumDuration( 0 );

    io_op_active_ = true;
    emit saveH5_();

    // TODO
    // Get file save progress indication from mcdriver
    int k = 0;
    QElapsedTimer tmr;
    tmr.start();
    while(io_op_active_) {
        dlg.setValue(k);
        if (tmr.hasExpired(250)) {
            k += 10;
            if (k>90) k=0;
            tmr.restart();
        }
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    if (io_ret_!=0) return false;

    setTemplate(false);
    setModified(false);

    return true;
}

void McDriverObj::start(bool b)
{
    if (is_running_) {
        if (!b) driver_->abort();
    } else if (b) {

        if (driver_->getSim() == nullptr) {
            mcdriver::options opt(options_);
            opt.Driver.max_no_ions = max_ions_;
            opt.Driver.seed = seed_;
            opt.Driver.threads = nThreads_;
            opt.Output.storage_interval = updInterval_;

            driver_->init(opt);

            info_.clear();

            emit simulationCreated();

            setStatus(mcIdle);

        } else {
            mcdriver::parameters par = driver_->driverOptions();
            par.max_no_ions = max_ions_;
            par.threads = nThreads_;
            driver_->setDriverOptions(par);

            mcdriver::output_options opts = driver_->outputOptions();
            opts.storage_interval = updInterval_;
            driver_->setOutputOptions(opts);
        }

        setModified(true);

        emit startSignal();
    }
}

void McDriverObj::start_()
{
    size_t updInterval = driver_->outputOptions().storage_interval;

    is_running_ = true;
    setStatus(mcRunning);

    info_.init(*this);

    emit simulationStarted(true);

    int ret = -1;
    if (driver_->getSim())
        ret = driver_->exec(mc_callback_,updInterval,this);

    is_running_ = false;

    info_.update(*this);

    emit simulationStarted(false);

    setStatus(mcIdle);
}

void McDriverObj::onLoadH5_(const QString &path)
{
    test_driver_ = new mcdriver;
    std::stringstream os;
    io_ret_ = test_driver_->load(path.toStdString(), &os);
    if (io_ret_ != 0) io_err_ = os.str();
    io_op_active_ = false;
}

void McDriverObj::onSaveH5_()
{    
    mcdriver::output_options opts = driver_->outputOptions();
    opts.outfilename = fileName_.toStdString();
    driver_->setOutputOptions(opts);

    std::stringstream os;
    io_ret_ = driver_->save(filePath_.toStdString(), &os);
    if (io_ret_ != 0) io_err_ = os.str();
    io_op_active_ = false;
}

void McDriverObj::setStatus(DriverStatus s)
{
    if (s != status()) {
        status_ = s;
        emit statusChanged();
    }
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
        setStatus(mcReset);
        emit simulationDestroyed();
        setModified(true);
    }
}


void McDriverObj::running_sim_info::clear()
{
    tstart_ = hr_clock_t::now();
    nstart_ = 0;
    ncurr_ = nstart_;
    ntarget_ = 0;
    progress_ = 0;
    total_elapsed_ = 0;
    elapsed_ = 0.;
    etc_ = 0;
    ips_ = 0.;
}

void McDriverObj::running_sim_info::init(const McDriverObj &D)
{
    tstart_ = hr_clock_t::now();
    nstart_ = D.driver_->getSim()->ion_count();
    ncurr_ = nstart_;
    ntarget_ = D.driver_->driverOptions().max_no_ions;
    progress_ = int(1000.0*ncurr_/ntarget_);
    total_elapsed_ += elapsed_;
    elapsed_ = 0.;
    etc_ = 0;
    ips_ = 0.;
}

void McDriverObj::running_sim_info::update(const McDriverObj &D)
{
    time_point t = hr_clock_t::now();
    ncurr_ = D.driver_->getSim()->ion_count();
    // floating-point duration: no duration_cast needed
    const std::chrono::duration<double> fp_sec = t - tstart_;
    elapsed_ = fp_sec.count();
    ips_ = (ncurr_ - nstart_)/elapsed_;
    etc_ = ips_ > 0 ? (ntarget_ - ncurr_)/ips_ : 0;
    progress_ = int(1000.0*ncurr_/ntarget_);
}
