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

    bar.update(d.getSim()->ion_count());
}

int main(int argc, char* argv[])
{   
    cxxopts::Options cli_options("iradina++", "Monte-Carlo ion trasport simulation");

    cli_options.add_options()
        ("n","Number of histories to run (overrides config input)", cxxopts::value<int>())
        ("j","Number of threads (overrides config input)", cxxopts::value<int>())
        ("s,seed","random generator seed (overrides config input)",cxxopts::value<int>())
        ("i,input","input HDF5 file name",cxxopts::value<std::string>())
        ("f","JSON config file",cxxopts::value<std::string>())
        ("o,output","output HDF5 file name (overrides config input)",cxxopts::value<std::string>())
        ("t,template","pring a template JSON config to stdout")
        ("v,version","Display version information")
        ("h,help","Display short help message");

    int n(-1), j(-1), s(-1);
    std::string input_config_file, input_file, output_file;

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
        if (result.count("f")) input_config_file = result["f"].as<std::string>();
        if (result.count("i")) input_file = result["i"].as<std::string>();
        if (result.count("o")) output_file = result["o"].as<std::string>();
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        cerr << "Error parsing command line: " << e.what() << endl;
        return -1;
    }

    if (input_config_file.empty() && input_file.empty()) {
        cerr << "Error: No input file." << endl;
        return -1;
    }

    if (!input_config_file.empty() && !input_file.empty()) {
        cerr << "Warning: JSON config ignored (HDF5 input will be used)." << endl;
        input_config_file.clear();
    }

    mcdriver D;

    if (!input_config_file.empty()) {

        cout << "Parsing JSON config from " << input_config_file << endl;

        std::ifstream is(input_config_file);
        mcdriver::options opt;
        if (opt.parseJSON(is,true,&cerr)!=0) return -1;

        // cli overrides
        if (n>0) opt.Driver.max_no_ions = n;
        if (j>0) opt.Driver.threads = j;
        if (s>0) opt.Driver.seed = s;
        if (!output_file.empty()) opt.Output.OutputFileBaseName = output_file;

        D.setOptions(opt);

    } else {

        cout << "Loading simulation from " << input_file << endl;

        if (D.load(input_file, &cerr)!=0) return -1;

        // cli overrides
        mcdriver::parameters par = D.driverOptions();
        if (n>0) par.max_no_ions = n;
        if (j>0) par.threads = j;
        D.setDriverOptions(par);

        mcdriver::output_options opts = D.outputOptions();
        if (!output_file.empty()) {
            opts.OutputFileBaseName = output_file;
            D.setOutputOptions(opts);
        }
    }

    cout << "Starting simulation '" << D.outputOptions().title << "'..." << endl << endl;

    bar.set_niter(D.driverOptions().max_no_ions);
    bar.update(D.getSim()->ion_count());

    D.exec(progress_callback,500);
    
    const mcdriver::run_data& rd = D.run_history().back();
    cout << endl << endl
         << "Completed " << rd.ion_count << " ion histories." << endl;
    cout << "ion/s = " << rd.ips << " (total), ";
    cout << rd.ips/rd.nthreads << " (per thread)" << endl;

    cout << "Storing results in " << D.outFileName() << " ...";
    cout.flush();
    D.save(D.outFileName(),&cerr);
    cout << " OK." << endl;

    return 0;
}





