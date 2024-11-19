#include "mccore.h"
#include "dedx.h"
#include "mcdriver.h"

#include <iomanip>
#include <iostream>
#include <highfive/H5Easy.hpp>
#include <highfive/highfive.hpp>

namespace h5e = H5Easy;
namespace h5= HighFive;

using std::cerr;
using std::endl;

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

int dump_event_stream(h5::File &h5f, const std::string &grp_name, const event_stream &es,
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

    std::ifstream ifs(es.fileName(), std::ios::binary);

    // copy data in chunks
    while (nrows)
    {
        count[0] = std::min(nrows, buff_rows); // # of rows to copy in this iter
        nrows -= count[0];

        // read from raw file buffer
        ifs.read((char *)buff.data(), count[0] * ncols * sizeof(float));

        // write to HDF5 file
        dataset.select(offset, count).write_raw<float>(buff.data());

        // advance offset
        offset[0] += count[0];
    }

    ifs.close();

    var_list << path << '\t'
             << shapeStr(dataset.getSpace()) << '\t'
             << dataset.getDataType().string() << '\t'
             << "event data" << endl;

    return 0;
}

int mcdriver::save()
{
try {    
    h5::File h5f(outFileName(), h5::File::Truncate);

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
    dump(h5f, "/run_stat/Nh", ion_count_, var_list, "# of histories");
    dump(h5f, "/run_stat/ips", ips_, var_list, "ion histories per second");
    dump(h5f, "/run_stat/process_cpu_time", ion_count_/ips_*opt.Driver.threads, var_list, "total cpu time [s]");
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
            ret = dump_array(h5f, name, t.at(k), dt.at(k), var_list, tally::arrayDescription(k), ion_count_)==0;
            k++;
        }

        dump_array(h5f, "/tally/totals/data", t.at(0), dt.at(0), var_list, "tally totals", ion_count_);
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
catch ( h5::Exception& e) {
    cerr << e.what() << endl;
    return -1;
}    

    return 0;
}
