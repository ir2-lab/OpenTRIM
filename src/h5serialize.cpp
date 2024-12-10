#include "mccore.h"
#include "dedx.h"
#include "mcdriver.h"

#include <iomanip>
#include <iostream>
#include <highfive/H5Easy.hpp>
#include <highfive/highfive.hpp>

namespace h5e = H5Easy;
namespace h5= HighFive;

#define FILE_TYPE_FIELD_NAME "FileType"
#define FILE_TYPE_FIELD_VALUE "IONS"
#define FILE_VERSION_MAJOR_FIELD_NAME "FileVersionMajor"
#define FILE_VERSION_MINOR_FIELD_NAME "FileVersionMinor"
#define FILE_VERSION_MAJOR_FIELD_VALUE 1
#define FILE_VERSION_MINOR_FIELD_VALUE 0


using std::cerr;
using std::endl;

int writeFileHeader(h5::File& file, std::ostream* os)
{
    try {
        file.createAttribute(FILE_TYPE_FIELD_NAME, std::string(FILE_TYPE_FIELD_VALUE));
        file.createAttribute(FILE_VERSION_MAJOR_FIELD_NAME, FILE_VERSION_MAJOR_FIELD_VALUE);
        file.createAttribute(FILE_VERSION_MINOR_FIELD_NAME, FILE_VERSION_MINOR_FIELD_VALUE);
    } catch ( h5::Exception& e) {
        if (os) (*os) << e.what() << endl;
        return -1;
    }
    return 0;
}

int readFileHeader(h5::File& file, int& VersionMajor, int& VersionMinor, std::ostream* os)
{
    try {
        h5::Attribute att = file.getAttribute(FILE_TYPE_FIELD_NAME);
        if (att.read<std::string>()!=std::string(FILE_TYPE_FIELD_VALUE)) {
            if (os) *os << "Incorrect file type";
            return -1;
        }
        att = file.getAttribute(FILE_VERSION_MAJOR_FIELD_NAME);
        VersionMajor = att.read<int>();
        att = file.getAttribute(FILE_VERSION_MINOR_FIELD_NAME);
        VersionMinor = att.read<int>();
    } catch ( h5::Exception& e) {
        if (os) (*os) << e.what() << endl;
        return -1;
    }
    return 0;
}

// create string describing the dataspace size, e.g. [10x20]
std::string shapeStr(const h5::DataSpace& dspace) {
    std::string ret;
    switch (H5Sget_simple_extent_type(dspace.getId()))
    {
    case H5S_SIMPLE:
        {
            std::vector<size_t> dims = dspace.getDimensions();
            ret = "[";
            ret += std::to_string(dims[0]);
            for(int i=1; i<dims.size(); i++) { ret += "x"; ret += std::to_string(dims[i]); }
            ret += "]";
        }
        break;
    case H5S_SCALAR:
        ret = "Scalar";
        break;
    case H5S_NULL:
        ret = "Empty";
        break;
    default:
        ret = "[?]";
        break;
    }
    return ret;
}

// dump data using H5Easy, create attribute with the description and write description to var_list
template <class T>
int dump(h5::File& file, const std::string& path, const T& data, 
         std::stringstream& var_list, const std::string& desc) {
    h5::DataSet dset = h5e::dump(file,path,data);
    dset.createAttribute("description",desc);
    var_list << path << '\t'
             << shapeStr(dset.getSpace()) << '\t'
             << dset.getDataType().string() << '\t'
             << desc << endl;
    return 0;
}

// dump data using H5Easy, create attribute with the description and write description to var_list
template <class T>
int dump(h5::File& file, const std::string& path, const T& data, const T& sem,
         std::stringstream& var_list, const std::string& desc) {
    h5::DataSet dset = h5e::dump(file,path,data);
    dset.createAttribute("description",desc);
    std::string buff = shapeStr(dset.getSpace());
    buff += '\t';
    buff += dset.getDataType().string();
    var_list << path << '\t'
             << buff << '\t'
             << desc << endl;
    dset = h5e::dump(file,path + "_sem", sem);
    dset.createAttribute("description",std::string("(SEM) ")+desc);
    var_list << path << "_sem" << '\t'
             << buff << '\t'
             << "(SEM) " << desc << endl;
    return 0;
}

template<typename T>
int dump_array(h5::File& file, const std::string& path,
               const ArrayND<T>& A, std::stringstream& var_list, 
               const std::string& desc,
               const size_t& N = 1)
{
    auto dset = file.createDataSet<T>(path, h5::DataSpace(A.dim()));
    dset.createAttribute("description",desc);
    const T* p = A.data();
    std::vector<T> a;
    if (N>1) {
        a.resize(A.size());
        for(size_t i=0; i<A.size(); i++) a[i] = A[i]/N;
        p = a.data();
    }
    dset.write_raw(p);

    var_list << path << '\t'
             << shapeStr(dset.getSpace()) << '\t'
             << dset.getDataType().string() << '\t'
             << desc << endl;
    return 0;
}

template<typename T>
int dump_array(h5::File& file, const std::string& path, 
            const ArrayND<T>& A, const ArrayND<T>& dA, 
            std::stringstream& var_list, const std::string& desc,
            const size_t& N)
{
    assert(A.size()==dA.size());
    assert(N>1);
    std::string dpath = path + "_sem";
    std::string ddesc("(SEM) ");
    ddesc += desc;
    ArrayND<T> M(A.dim()), S(A.dim()); // make a buffer arrays initialized to 0
    for(int i=0; i<A.size(); i++) {
        M[i] = A[i]/N;
        // error in the mean
        S[i] = std::sqrt((dA[i]/N-M[i]*M[i])/(N-1));
    }
    return dump_array(file,  path, M, var_list,  desc) + 
           dump_array(file, dpath, S, var_list, ddesc);
}

template<typename T>
int load_array(h5::File& file, const std::string& path,
               ArrayND<T>& A, const size_t& N = 1)
{
    auto dset = file.getDataSet(path);

    assert(dset.getDimensions() == A.dim());

    dset.read(A.data());

    for(size_t i=0; i<A.size(); i++) A[i] *= N;

    return 0;
}

template<typename T>
int load_array(h5::File& file, const std::string& path,
               ArrayND<T>& A, ArrayND<T>& dA,
               const size_t& N)
{
    assert(A.size()==dA.size());
    assert(N>1);
    std::string dpath = path + "_sem";

    int ret = load_array(file,  path,  A, 1) +
           load_array(file, dpath, dA, 1);

    if (ret!=0) return ret;

    for(size_t i=0; i<A.size(); i++)
    {
        dA[i] = N*(A[i]*A[i] + (N-1)*dA[i]*dA[i]);
        A[i] *= N;
    }

    return 0;
}

int dump_event_stream(h5::File &h5f, const std::string &grp_name, event_stream &es,
                      std::stringstream &var_list)
{
    // get row, column numbers
    size_t nrows(es.rows()), ncols(es.cols());

    std::string path;
    path = grp_name + "/column_names";
    dump(h5f, path, es.event_prototype().columnNames(), var_list, "event data column names");
    path = grp_name + "/column_descriptions";
    dump(h5f, path, es.event_prototype().columnDescriptions(), var_list, "event data column descriptions");

    path = grp_name + "/event_data";

    if (nrows==0) {
        // No data. Create empty dataset and leave
        h5::DataSet dataset = h5f.createDataSet<float>(path, h5::DataSpace(nrows,ncols));
        var_list << path << '\t'
                 << shapeStr(dataset.getSpace()) << '\t'
                 << dataset.getDataType().string() << '\t'
                 << "event data" << endl;
        return 0;
    }

    // mem buffer ~1MB
    size_t buff_rows = std::ceil(1.*(1 << 20)/4/ncols);
    buff_rows = std::min(buff_rows, nrows);
    std::vector<float> buff(buff_rows * ncols);
    
    // Create the dataset.
    // Use compression + chunking
    h5::DataSetCreateProps dscp;
    dscp.add(h5::Deflate(6));
    dscp.add(h5::Chunking({buff_rows,ncols}));
    h5::DataSet dataset = h5f.createDataSet<float>(path, h5::DataSpace(nrows,ncols),dscp);

    std::vector<size_t> offset{0, 0};
    std::vector<size_t> count{buff_rows, ncols};

    es.rewind();

    // copy data in chunks
    while (nrows)
    {
        count[0] = std::min(nrows, buff_rows); // # of rows to copy in this iter
        nrows -= count[0];

        // read from raw file buffer
        es.read(buff.data(), count[0]);

        // write to HDF5 file
        dataset.select(offset, count).write_raw<float>(buff.data());

        // advance offset
        offset[0] += count[0];
    }

    var_list << path << '\t'
             << shapeStr(dataset.getSpace()) << '\t'
             << dataset.getDataType().string() << '\t'
             << "event data" << endl;

    return 0;
}

int load_event_stream(h5::File &h5f, const std::string &grp_name, event_stream &es)
{
    // get row, column numbers
    size_t ncols(es.cols());

    std::string path;
    path = grp_name + "/event_data";

    // Get the dataset.
    h5::DataSet dataset = h5f.getDataSet(path);

    std::vector<size_t> dims = dataset.getSpace().getDimensions();
    size_t nrows = dims[0];
    assert(ncols==dims[1]);

    // mem buffer ~1MB
    size_t buff_rows = std::ceil(1.*(1 << 20)/4/ncols);
    buff_rows = std::min(buff_rows, nrows);
    std::vector<float> buff(buff_rows * ncols);

    std::vector<size_t> offset{0, 0};
    std::vector<size_t> count{buff_rows, ncols};

    es.rewind();

    // copy data in chunks
    while (nrows)
    {
        count[0] = std::min(nrows, buff_rows); // # of rows to copy in this iter
        nrows -= count[0];

        // read from HDF5 file
        dataset.select(offset, count).read<float>(buff.data());

        // write to raw file buffer
        es.write(buff.data(), count[0]);

        // advance offset
        offset[0] += count[0];
    }

    return 0;
}

int mcdriver::save(const std::string &h5filename, std::ostream *os)
{
try {    
    h5::File h5f(h5filename, h5::File::Truncate);

    if (writeFileHeader(h5f,os)!=0) return -1;

    options opt;
    getOptions(opt);

    // variable list
    std::stringstream var_list;

    // title
    dump(h5f, "title", opt.Output.title, var_list, "user supplied simulation title");

    // save options
    {
        std::stringstream ss;
        opt.printJSON(ss);
        dump(h5f, "config_json", ss.str(), var_list, "JSON formatted simulation options");
    }

    // save iradina++ version info
    var_list << "Version Info" << endl;
    dump(h5f, "/version_info/version", std::string(IRADINAPP_VERSION),var_list,"iradina++ version");
    dump(h5f, "/version_info/compiler", std::string(COMPILER_ID),var_list,"compiler id");
    dump(h5f, "/version_info/compiler_version", std::string(COMPILER_VERSION),var_list,"compiler version");
    dump(h5f, "/version_info/build_system", std::string(SYSTEM_ID),var_list,"build system");
    dump(h5f, "/version_info/build_time", std::string(BUILD_TIME),var_list,"build timestamp");
    var_list << endl;

    // save run statistics
    var_list << "Run statistics" << endl;
    const tally& t = s_->getTally();
    const tally& dt = s_->getTallyVar();
    dump(h5f, "/run_stat/Nh", getSim()->ion_count(), var_list, "# of histories");
    dump(h5f, "/run_stat/ips", ips_, var_list, "ion histories per second");
    dump(h5f, "/run_stat/process_cpu_time", cpu_time_, var_list, "total cpu time [s]");
    {
        std::stringstream ss;
        ss << std::put_time(std::localtime(&start_time_), "%c %Z");
        dump(h5f, "/run_stat/start_time", ss.str(), var_list, "start time/date");
    }
    {
        std::stringstream ss;
        ss << std::put_time(std::localtime(&end_time_), "%c %Z");
        dump(h5f, "/run_stat/end_time", ss.str(), var_list, "finish time/date");
    }
    var_list << endl;

    // save grid
    var_list << "Spatial Grid" << endl;
    auto grid = s_->getTarget().grid();
    dump(h5f, "/grid/X", dynamic_cast<const std::vector<float>&>(grid.x()),var_list,"x-axis grid");
    dump(h5f, "/grid/Y", dynamic_cast<const std::vector<float>&>(grid.y()),var_list,"y-axis grid");
    dump(h5f, "/grid/Z", dynamic_cast<const std::vector<float>&>(grid.z()),var_list,"z-axis grid");
    { // save xyz of each cell center
        int rows = grid.x().size()-1;
        int cols = grid.y().size()-1;
        int layers = grid.z().size()-1;
        ArrayND<float> buff(3,grid.ncells());

        for(int i=0; i<rows; i++)
            for(int j=0; j<cols; j++)
                for(int k=0; k<layers; k++)
                {
                    int l = (i*cols+j)*layers+k;
                    buff(0,l) = 0.5f*(grid.x()[i] + grid.x()[i+1]);
                    buff(1,l) = 0.5f*(grid.y()[j] + grid.y()[j+1]);
                    buff(2,l) = 0.5f*(grid.z()[k] + grid.z()[k+1]);
                }

        dump_array(h5f, "/grid/cell_xyz", buff, var_list, "cell center coordinates");
    }
    var_list << endl;

    // save atoms & materials
    var_list << "Atom data" << endl;
    auto atoms = s_->getTarget().atoms();
    std::vector<std::string> atom_labels = s_->getTarget().atom_labels();
    dump(h5f, "/atom/label", atom_labels, var_list, "labels = [Atom (Chemical name)] in [Material]");
    for(int i=0; i<atoms.size(); i++) atom_labels[i] = atoms[i]->name();
    dump(h5f, "/atom/name", atom_labels, var_list, "Chemical names");
    {
        std::vector<float> A(atoms.size());
        for(int i=0; i<atoms.size(); i++) A[i] = atoms[i]->Z();
        dump(h5f, "/atom/Z", A, var_list, "atomic number");
        for(int i=0; i<atoms.size(); i++) A[i] = atoms[i]->M();
        dump(h5f, "/atom/M", A, var_list, "atomic mass");
        for(int i=0; i<atoms.size(); i++) A[i] = atoms[i]->Ed();
        dump(h5f, "/atom/Ed", A, var_list, "displacement energy [eV]");
        for(int i=0; i<atoms.size(); i++) A[i] = atoms[i]->El();
        dump(h5f, "/atom/El", A, var_list, "lattice binding energy [eV]");
        for(int i=0; i<atoms.size(); i++) A[i] = atoms[i]->Es();
        dump(h5f, "/atom/Es", A, var_list, "surface binding energy [eV]");
        for(int i=0; i<atoms.size(); i++) A[i] = atoms[i]->Er();
        dump(h5f, "/atom/Er", A, var_list, "replacement energy [eV]");
    }
    var_list << endl;
    var_list << "Materials" << endl;
    auto mat = s_->getTarget().materials();
    {
        std::vector<std::string> name;
        std::vector<float> nat,nm,rat;
        for(auto m : mat) {
            name.push_back(m->name());
            nat.push_back(m->atomicDensity());
            nm.push_back(m->massDensity());
            rat.push_back(m->atomicRadius());
        }
        dump(h5f, "/material/name", name, var_list, "name of material");
        dump(h5f, "/material/atomic_density", nat, var_list, "atomic density [at/nm^3]");
        dump(h5f, "/material/mass_density", nm, var_list, "mass density [g/cm^3]");
        dump(h5f, "/material/atomic_radius", rat, var_list, "atomic radius [nm]");
    }
    var_list << endl;

    // ion beam data
    var_list << "Ion Beam" << endl;
    {
        auto p = s_->getSource().getParameters();
        dump(h5f, "/ion_beam/E0", p.ionE0, var_list, "ion energy [eV]");
        dump(h5f, "/ion_beam/Z", p.ionZ, var_list, "ion atomic number");
        dump(h5f, "/ion_beam/M", p.ionM, var_list, "ion mass [amu]");
    }

    // save tallys
    var_list << "Tallies" << endl;
    var_list << "  Results are mean values over the ion histories. " << endl
             << "  [VarName]_sem is the Standard Error of the Mean (SEM) for the quantity [VarName]." << endl;
    {
        bool ret = true;
        int k = 1;

        while(ret && k<tally::std_tallies) {
            std::string name("/tally/");           
            name += tally::arrayGroup(k);
            name += "/";
            name += tally::arrayName(k);
            ret = dump_array(h5f, name, t.at(k), dt.at(k), var_list, tally::arrayDescription(k), getSim()->ion_count())==0;
            k++;
        }

        dump_array(h5f, "/tally/totals/data", t.at(0), dt.at(0), var_list, "tally totals", getSim()->ion_count());
        dump(h5f, "/tally/totals/column_names", t.arrayNames(), var_list, "names of totals");

        if (!ret) return -1;

    }

    if (opt.Output.store_dedx) {
        var_list << endl << "Electronic energy loss and straggling tables" << endl;

        ArrayNDf A(dedx_index::size);
        for(dedx_index i; i<i.end(); i++) A(i) = *i;
        dump_array(h5f,"/eels/dEdx_erg",A,var_list,"dEdx table energy values [eV]");

        ArrayND< dedx_interp* > D = s_->dedx();
        A = ArrayNDf(D.dim()[0],D.dim()[1],dedx_index::size);
        for(int i=0; i<A.dim()[0]; i++)
            for(int j=0; j<A.dim()[1]; j++)
                memcpy(&A(i,j,0),D(i,j)->data(),dedx_index::size*sizeof(float));
        dump_array(h5f,"/eels/dEdx",A,var_list,"dEdx values [eV/nm]");

        ArrayND< straggling_interp* > Ds = s_->de_strag();
        for(int i=0; i<A.dim()[0]; i++)
            for(int j=0; j<A.dim()[1]; j++)
                memcpy(&A(i,j,0),Ds(i,j)->data(),dedx_index::size*sizeof(float));
        dump_array(h5f,"/eels/dEstrag",A,var_list,"straggling values [eV]");

        dump_array(h5f,"/eels/mfp",s_->mfp(),var_list,"ion mean free [nm]");
        dump_array(h5f,"/eels/ipmax",s_->ipmax(),var_list,"max impact parameter [nm]");
        dump_array(h5f,"/eels/fpmax",s_->fpmax(),var_list,"max free path [nm]");
        dump_array(h5f,"/eels/Tcutoff",s_->Tcutoff(),var_list,"recoil energy cut0ff [eV]");
    }

    if (opt.Output.store_pka || opt.Output.store_transmitted_ions)
        var_list << endl << "Event data" << endl;
    if (opt.Output.store_pka) dump_event_stream(h5f,"/pka_events",s_->pka_stream(),var_list);
    if (opt.Output.store_transmitted_ions) dump_event_stream(h5f,"/exit_events",s_->exit_stream(),var_list);

    var_list << endl;

    // Save the rng state
    auto rngstate = s_->rngState();
    // make a copy, because std::array is not supported
    std::vector<random_vars::result_type> state_cpy(rngstate.size());
    for(int i=0; i<rngstate.size(); ++i) state_cpy[i] = rngstate[i];
    dump(h5f,"/rng_state",state_cpy,var_list,"random generator state");

    h5e::dump(h5f, "variable_list", var_list.str());

}
catch (h5::Exception& e) {
    if (os) (*os) << e.what() << endl;
    return -1;
}

return 0;
}

int mcdriver::load(const std::string &h5filename, std::ostream *os)
{
    if (s_) {
        if (os) (*os) << "Reset the simulation object before calling load";
        return -1;
    }

    try {
        h5::File h5f(h5filename, h5::File::ReadOnly);

        int vMajor, vMinor;
        if (readFileHeader(h5f, vMajor, vMinor, os)!=0) return -1;
        // TODO
        // In future versions check file version

        // load and check simulation options
        options opt;
        {
            std::string json = h5e::load<std::string>(h5f,"config_json");
            std::stringstream is(json);
            if (opt.parseJSON(is,true,os)!=0) return -1;
        }

        // set options in driver. simulation object is created
        setOptions(opt);

        // Load the rng state
        {
            // get a copy of the current state, as a std::array
            auto rngstate = s_->rngState();
            // load a vector, because std::array is not supported
            std::vector<random_vars::result_type> state_cpy =
                h5e::load< std::vector<random_vars::result_type> >(h5f, "/rng_state");
            // copy into std::array
            for(int i=0; i<state_cpy.size(); ++i) rngstate[i] = state_cpy[i];
            // set it in simulation object
            s_->setRngState(rngstate);
        }

        // load the tally data
        {
            size_t Nh = h5e::load<size_t>(h5f,"/run_stat/Nh");
            ips_ = h5e::load<double>(h5f,"/run_stat/ips");

            bool ret = true;
            int k = 1;

            tally& t = s_->getTally();
            tally& dt = s_->getTallyVar();

            while(ret && k<tally::std_tallies) {
                std::string name("/tally/");
                name += tally::arrayGroup(k);
                name += "/";
                name += tally::arrayName(k);
                ret = load_array(h5f, name, t.at(k), dt.at(k), Nh)==0;
                k++;
            }

            ret = ret &&
                  load_array(h5f, "/tally/totals/data", t.at(0), dt.at(0), Nh)==0;

            if (!ret) return -1;

            s_->setIonCount(Nh);
        }

        // load pka events
        if (opt.Output.store_pka)
        {
            s_->open_pka_stream();
            load_event_stream(h5f,"/pka_events",s_->pka_stream());

        }

        // load pka events
        if (opt.Output.store_transmitted_ions)
        {
            s_->open_exit_stream();
            load_event_stream(h5f,"/exit_events",s_->exit_stream());

        }

    } catch ( h5::Exception& e) {
        if (os) (*os) << e.what() << endl;
        return -1;
    }

    return 0;
}
