#include "mcdriver.h"

#include <cxxopts.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define WINDOWS_BUILD
	#include <Windows.h>
	#include <cstdio>
#endif

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

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
    int progress_; // 0..pmax
    double elapsed_; // elapsed time in s
    double etc_; // estimated time to completion (s)
    double ips_; // ions per second

    // cli progress bar
    // # of char blocks
    const static int n_blocks_ = 40;
    // max value of progress_
    const static int max_progress_ = n_blocks_ << 3;

public:
    // init : called before simulation starts
    void init(const mcdriver &D);
    // update : called during simulation run
    void update(const mcdriver &D);
    // print cli progress bar
    void print();
    // getters
    int progress() const { return progress_; }
    double elapsed() const { return elapsed_; }
    double ips() const { return ips_; }
    double etc() const { return etc_; }
    size_t nions() const { return ncurr_; }
};

running_sim_info info;

void progress_callback(const mcdriver &d, void *)
{
    info.update(d);
    info.print();
}

int main(int argc, char *argv[])
{
	
#ifdef WINDOWS_BUILD
	// https://stackoverflow.com/questions/45575863/how-to-print-utf-8-strings-to-stdcout-on-windows
	// Set console code page to UTF-8 so console known how to interpret string data
    SetConsoleOutputCP(CP_UTF8);

    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

    std::string program_name(PROJECT_NAME);
    std::transform(program_name.begin(), program_name.end(), program_name.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    cxxopts::Options cli_options(program_name, PROJECT_DESCRIPTION);

    cli_options.add_options()("n", "Number of histories to run (overrides config input)",
                              cxxopts::value<int>())(
            "j", "Number of threads (overrides config input)", cxxopts::value<int>())(
            "s,seed", "random generator seed (overrides config input)", cxxopts::value<int>())(
            "i,input", "input HDF5 file name",
            cxxopts::value<std::string>())("f", "JSON config file", cxxopts::value<std::string>())(
            "o,output", "output HDF5 file name (overrides config input)",
            cxxopts::value<std::string>())("t,template", "print a template JSON config to stdout")(
            "v,version", "Display version information")("h,help", "Display short help message");

    int n(-1), j(-1), s(-1);
    std::string input_config_file, input_file, output_file;

    try {
        auto result = cli_options.parse(argc, argv);

        if (result.count("help")) {
            cout << cli_options.help() << endl;
            return 0;
        }
        if (result.count("version")) {
            cout << PROJECT_NAME << " version " << PROJECT_VERSION << endl;
            cout << "Build time: " << BUILD_TIME << endl;
            cout << "Compiler: " << COMPILER_ID << " v" << COMPILER_VERSION << " on " SYSTEM_ID
                 << endl;
            return 0;
        }
        if (result.count("template")) {
            mcdriver::options opt;
            opt.printJSON(cout);
            return 0;
        }
        if (result.count("n"))
            n = result["n"].as<int>();
        if (result.count("j"))
            j = result["j"].as<int>();
        if (result.count("s"))
            s = result["s"].as<int>();
        if (result.count("f"))
            input_config_file = result["f"].as<std::string>();
        if (result.count("i"))
            input_file = result["i"].as<std::string>();
        if (result.count("o"))
            output_file = result["o"].as<std::string>();
    } catch (const cxxopts::exceptions::exception &e) {
        cerr << "Error parsing command line: " << e.what() << endl;
        return -1;
    }

    if (!input_config_file.empty() && !input_file.empty()) {
        cerr << "Warning: JSON config ignored (HDF5 input will be used)." << endl;
        input_config_file.clear();
    }

    mcdriver D;

    if (!input_file.empty()) {

        cout << "Loading simulation from " << input_file << endl;

        if (D.load(input_file, &cerr) != 0)
            return -1;

        // cli overrides
        mcdriver::parameters par = D.driverOptions();
        if (n > 0)
            par.max_no_ions = n;
        if (j > 0)
            par.threads = j;
        D.setDriverOptions(par);

        mcdriver::output_options opts = D.outputOptions();
        if (!output_file.empty()) {
            opts.outfilename = output_file;
            D.setOutputOptions(opts);
        }
    } else {

        mcdriver::options opt;

        if (!input_config_file.empty()) {

            cout << "Parsing JSON config from " << input_config_file << endl;

            std::ifstream is(input_config_file);

            if (opt.parseJSON(is, true, &cerr) != 0)
                return -1;
        } else {
            if (opt.parseJSON(cin, true, &cerr) != 0)
                return -1;
        }

        // cli overrides
        if (n > 0)
            opt.Driver.max_no_ions = n;
        if (j > 0)
            opt.Driver.threads = j;
        if (s > 0)
            opt.Driver.seed = s;
        if (!output_file.empty())
            opt.Output.outfilename = output_file;

        D.init(opt);
    }

    cout << "Starting simulation '" << D.outputOptions().title << "'..." << endl << endl;

    info.init(D);
    info.print();

    D.exec(progress_callback, 200);

    const mcdriver::run_data &rd = D.run_history().back();
    cout << endl << endl << "Completed " << rd.total_ion_count << " ion histories." << endl;
    cout << "Threads: " << D.driverOptions().threads << endl;
    cout << "Cpu time (s):  " << rd.cpu_time << ",\t" << "Ions/cpu-s:  " << rd.ips << endl;
    cout << "Real time (s): " << info.elapsed() << ",\t" << "Ions/real-s: " << info.ips() << endl;
    cout << "Storing results in " << D.outFileName() << " ...";
    cout.flush();
    D.save(D.outFileName(), &cerr);
    cout << " OK." << endl;

    return 0;
}

void running_sim_info::init(const mcdriver &d)
{
    tstart_ = hr_clock_t::now();
    nstart_ = d.getSim()->ion_count();
    ncurr_ = nstart_;
    ntarget_ = d.driverOptions().max_no_ions;
    progress_ = int(1.0 * max_progress_ * ncurr_ / ntarget_);
    elapsed_ = 0.;
    etc_ = 0;
    ips_ = 0.;
}

void running_sim_info::update(const mcdriver &d)
{
    time_point t = hr_clock_t::now();
    ncurr_ = d.getSim()->ion_count();
    // floating-point duration: no duration_cast needed
    const std::chrono::duration<double> fp_sec = t - tstart_;
    elapsed_ = fp_sec.count();
    ips_ = (ncurr_ - nstart_) / elapsed_;
    etc_ = ips_ > 0 ? (ntarget_ - ncurr_) / ips_ : 0;
    progress_ = int(1.0 * max_progress_ * ncurr_ / ntarget_);
}

// format time t (s) as hh:mm:ss
const char *mytimefmt_(double t, bool ceil = false)
{
    static char buff[32];

    std::ldiv_t dv;
    dv.quot = ceil ? std::ceil(t) : std::floor(t);
    dv = ldiv(dv.quot, 60L);
    long s = dv.rem;
    dv = ldiv(dv.quot, 60L);
    long m = dv.rem;

    sprintf(buff, "%02ld:%02ld:%02ld", dv.quot, m, s);
    return buff;
}

// print to console the progress bar, progress &
// ETC = estimated time of completion
#ifdef WINDOWS_BUILD  // on windows console use only full block char and no color

void running_sim_info::print()
{
    /*
     * Unicode left block characters, full and half
     * - UTF-8 encoding, each char is 3 bytes
     */
    static const char block_chars[] = u8"█\0▌";
    // separator char
    static const char sep[] = u8"║";

    cout << '\r'; // clear the line
    cout << sep;

    // 0 <= progress <= pmax
    // 1 full block = pmax/8 = 2.5%
    int n = progress_ >> 3; // # of full blocks
    int m = (progress_ & 0x07) > 0x03; // # of 1/8ths for the last block

    // add n #
    for (int j = 0; j < n; ++j)
        std::cout << block_chars;

    if (m) {
        // get a pointer to a partially filled char
        const char *ws = block_chars + 4;
        std::cout << ws;
        n++;
    }

    // add spaces
    for (int j = 0; j < n_blocks_ - n; ++j)
        cout << ' ';

    cout << sep;

    // print progress percentage
    char buff[8];
    sprintf(buff, "%3d%%", progress_ * 100 / max_progress_);
    cout << buff;

    cout << sep;

    // print ETC
    cout << "ETC " << mytimefmt_(etc_, true);

    cout << sep;

    cout.flush();
}

#else // on linux use color and fancy UTF-8 chars

void running_sim_info::print()
{
    /*
     * Unicode left block characters, fill ratio 1/8 to 8/8
     * - UTF-8 encoding, each char is 3 bytes
     * - Each char in block_chars[] is separated by a '0'
     * - block_chars[0] is the full block char
     * - block_chars[4*m], m=1..7, is the character with fill ratio m/8
     *
     * TODO: check in windows build
     */
    static const char block_chars[] = u8"█\0▏\0▎\0▍\0▌\0▋\0▊\0▉";
    // separator char
    static const char sep[] = u8"║";

    cout << '\r'; // clear the line
    cout << sep;

    // 0 <= progress <= pmax
    // 1 full block = pmax/8 = 2.5%
    int n = progress_ >> 3; // # of full blocks
    int m = progress_ & 0x07; // # of 1/8ths for the last block

    int textColor = 90; // "bright black"
    cout << "\033[" << textColor << "m";

    // add n #
    for (int j = 0; j < n; ++j)
        std::cout << block_chars;

    if (m) {
        // get a pointer to a partially filled char
        const char *ws = block_chars + (m << 2);
        std::cout << ws;
        n++;
    }
	
    // reset color
    cout << "\033[0m";

    // add spaces
    for (int j = 0; j < n_blocks_ - n; ++j)
        cout << ' ';

    cout << sep;

    // print progress percentage
    char buff[16];
    sprintf(buff, "%3d%%", progress_ * 100 / max_progress_);
    cout << buff;

    cout << sep;

    // print ETC
    cout << "ETC " << mytimefmt_(etc_, true);

    cout << sep;

    cout.flush();
}

#endif
