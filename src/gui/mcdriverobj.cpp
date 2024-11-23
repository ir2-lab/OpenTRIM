#include "mcdriverobj.h"

#include "mcdriver.h"
#include "qjsonpath/qjsonpath.h"

#include <fstream>

#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QFileInfo>


// McDriverObj should not have a parent
// because it lives in a separate thread
McDriverObj::McDriverObj()
    : QObject(),
    driver_(new mcdriver),
    modified_(false),
    is_running_(false),
    status_(0)
{
    // internal start signal connection
    connect(this, &McDriverObj::startSignal,
            this, &McDriverObj::start_,Qt::QueuedConnection);

}

McDriverObj::~McDriverObj()
{
    delete driver_;
}

void McDriverObj::setJsonOptions(const QJsonDocument &jdoc)
{
    jsonOptions_ = jdoc;
    emit configChanged();
    setModified(true);
}

void McDriverObj::setModified(bool b)
{
    if (modified_ != b) {
        modified_ = b;
        emit modificationChanged(b);
    }
}

QString McDriverObj::fileName() const
{
    return QJsonPath::get(jsonOptions_, "Output/OutputFileBaseName").toString();
}

void McDriverObj::setFileName(const QString &s)
{
    if (s != fileName()) {
        mcdriver::output_options opts = driver_->outputOptions();
        opts.OutputFileBaseName = s.toLatin1().toStdString();
        driver_->setOutputOptions(opts);
        QJsonPath::set(jsonOptions_, "Output/OutputFileBaseName", s);
        emit fileNameChanged();
    }
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

bool McDriverObj::validateOptions(QString* msg) const
{
    mcdriver::options opt;

    std::string s(jsonOptions_.toJson().constData());
    std::stringstream ss(s, std::ios_base::in);

    int ret = opt.parseJSON(ss,false);

    if (ret) {
        if (msg) *msg = "Internal error in JSON opts !!";
        return false;
    }

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

    // In any case, this is called Untitled
    opt.Output.OutputFileBaseName = "Untitled";
    template_ = true;

    std::stringstream os;
    opt.printJSON(os);
    jsonOptions_ = QJsonDocument::fromJson(os.str().c_str());

    setModified(false);

    emit configChanged();
    emit contentsChanged();
    emit fileNameChanged();
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

    QFileInfo finfo(path);
    opt.Output.OutputFileBaseName = finfo.baseName().toStdString();
    template_ = false;

    std::stringstream os;
    opt.printJSON(os);
    jsonOptions_ = QJsonDocument::fromJson(os.str().c_str());

    setModified(false);

    emit configChanged();
    emit contentsChanged();
    emit fileNameChanged();
}

void McDriverObj::saveJson(const QString &fname)
{
    QFileInfo finfo(fname);
    setFileName(finfo.baseName());

    mcdriver::options opt;
    if (driver_->getSim())
        driver_->getOptions(opt); // get options from mccore object
    else { // get from our local json
        std::istringstream is(jsonOptions_.toJson().constData());
        opt.parseJSON(is,false);
    }
    std::ofstream os(fname.toLatin1().constData());
    opt.printJSON(os);

    template_ = false;

    if (!driver_->getSim()) setModified(false);
}

void McDriverObj::saveH5(const QString &fname)
{
    if (!driver_->getSim()) return;

    QFileInfo finfo(fname);
    setFileName(finfo.baseName());

    driver_->save();

    template_ = false;

    setModified(false);
}

void McDriverObj::start(bool b)
{
    if (is_running_) {
        if (!b) driver_->abort();
    } else if (b) {

        if (driver_->getSim() == nullptr) {
            mcdriver::options opt;
            std::string s(jsonOptions_.toJson().constData());
            std::stringstream ss(s, std::ios_base::in);
            opt.parseJSON(ss,false);
            opt.Driver.max_no_ions = max_ions_;
            opt.Driver.seed = seed_;
            opt.Driver.threads = nThreads_;
            opt.Output.storage_interval = updInterval_;

            driver_->setOptions(opt);

            total_elapsed_ = 0;

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

    init_run_data();

    emit simulationStarted(true);

    int ret = -1;
    if (driver_->getSim())
        ret = driver_->exec(mc_callback_,updInterval,this);

    is_running_ = false;
    update_run_data();
    eta_ = 0;
    total_elapsed_ += elapsed_;
    elapsed_ = 0.;

    emit simulationStarted(false);

    setStatus(mcIdle);
}

void McDriverObj::setStatus(DriverStatus s)
{
    if (s != status()) {
        status_ = s;
        emit statusChanged();
    }

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
        setStatus(mcReset);
        emit simulationDestroyed();
        setModified(true);
    }
}


