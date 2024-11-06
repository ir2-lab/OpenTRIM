#include "mcdriver.h"

#include <cxxopts.hpp>
#include <progressbar.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

progressbar bar;

void progress_callback(const mcdriver& d, void* )
{
//    const std::vector<size_t>&  c = d.thread_ion_count();
//    size_t ct = d.ion_count();
//    cout << c[0];
//    for(int i=1; i<c.size(); i++) cout << '\t' << c[i];
//    cout << '\t' << ct
//         << '\t'  << d.getSim()->ion_que_size() << endl;

    bar.update(d.ion_count());
}

int main(int argc, char* argv[])
{   
    cxxopts::Options cli_options("iradina++", "Monte-Carlo ion trasport simulation");

    cli_options.add_options()
        ("n","Number of histories to run (overrides config input)", cxxopts::value<int>())
        ("j","Number of threads (overrides config input)", cxxopts::value<int>())
        ("s,seed","random generator seed (overrides config input)",cxxopts::value<int>())
        ("o,output","output file base name (overrides config input)",cxxopts::value<std::string>())
        ("f","JSON config file",cxxopts::value<std::string>())
        ("t,template","pring a template JSON config to stdout")
        ("v,version","Display version information")
        ("h,help","Display short help message");

    int n(-1), j(-1), s(-1);
    std::string input_file, output_file;

    try {
        auto result = cli_options.parse(argc,argv);

        if (result.count("help")) {
            cout << cli_options.help() << endl;
            return 0;
        }
        if (result.count("version")) {
            cout << "iradina++ version " << IRADINAPP_VERSION << endl;
            cout << "Build time: " << BUILD_TIME << endl;
            cout << "Compiler: " << COMPILER_ID << " v" << COMPILER_VERSION << " on " SYSTEM_ID << endl;
            return 0;
        }
        if (result.count("template")) {
            mcdriver::options opt;
            opt.printJSON(cout);
            return 0;
        }
        if (result.count("n")) n = result["n"].as<int>();
        if (result.count("j")) j = result["j"].as<int>();
        if (result.count("s")) s = result["s"].as<int>();
        if (result.count("f")) input_file = result["f"].as<std::string>();
        if (result.count("o")) output_file = result["o"].as<std::string>();
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        cout << "error parsing options: " << e.what() << endl;
        return -1;
    }

    // Parse JSON config
    mcdriver::options opt;
    if (input_file.empty()) {
        cout << "Input JSON config:" << endl;
        if (opt.parseJSON(cin)!=0) return -1;
    } else {
        cout << "Parsing JSON config from " << input_file << endl;
        std::ifstream is(input_file);
        if (opt.parseJSON(is)!=0) return -1;
    }

    // cli overrides
    if (n>0) opt.Driver.max_no_ions = n;
    if (j>0) opt.Driver.threads = j;
    if (!output_file.empty()) opt.Output.OutputFileBaseName = output_file;
    /// @todo Fix the seed cli option for iradina++
    /// if (s>0) opt.Driver.seeds = s;

    cout << "Starting simulation '" << opt.Output.title << "'..." << endl << endl;
    bar.set_niter(opt.Driver.max_no_ions);
    bar.update(0);
    mcdriver D;
    D.setOptions(opt);
    D.exec(progress_callback,500);
    
    tally t = D.getSim()->getTally();
    cout << endl << endl
         << "Completed " << t.Nions() << " ion histories." << endl;
    cout << "ion/s = " << D.ips() << " (total), ";
    cout << D.ips()/D.nThreads() << " (per thread)" << endl;

    cout << "Storing results in " << D.outFileName() << " ...";
    cout.flush();
    D.save();
    cout << " OK." << endl;

    return 0;
}





