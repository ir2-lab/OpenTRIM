#include "mccore.h"
#include "mcdriver.h"
#include "mcinfo.h"

#include <iostream>
#include <highfive/H5Easy.hpp>
#include <highfive/highfive.hpp>

#include "json_defs_p.h"

namespace h5e = H5Easy;
namespace h5 = HighFive;

#define FILE_TYPE_FIELD_NAME "FileType"
#define FILE_TYPE_FIELD_VALUE "OpenTRIM"
#define FILE_VERSION_MAJOR_FIELD_NAME "FileVersionMajor"
#define FILE_VERSION_MINOR_FIELD_NAME "FileVersionMinor"
#define FILE_VERSION_MAJOR_FIELD_VALUE 1
#define FILE_VERSION_MINOR_FIELD_VALUE 0

using std::cerr;
using std::endl;

int writeFileHeader(h5::File &file, std::ostream *os)
{
    try {
        file.createAttribute(FILE_TYPE_FIELD_NAME, std::string(FILE_TYPE_FIELD_VALUE));
        file.createAttribute(FILE_VERSION_MAJOR_FIELD_NAME, FILE_VERSION_MAJOR_FIELD_VALUE);
        file.createAttribute(FILE_VERSION_MINOR_FIELD_NAME, FILE_VERSION_MINOR_FIELD_VALUE);
    } catch (h5::Exception &e) {
        if (os)
            (*os) << e.what() << endl;
        return -1;
    }
    return 0;
}

int readFileHeader(h5::File &file, int &VersionMajor, int &VersionMinor, std::ostream *os)
{
    try {
        h5::Attribute att = file.getAttribute(FILE_TYPE_FIELD_NAME);
        if (att.read<std::string>() != std::string(FILE_TYPE_FIELD_VALUE)) {
            if (os)
                *os << "Incorrect file type";
            return -1;
        }
        att = file.getAttribute(FILE_VERSION_MAJOR_FIELD_NAME);
        VersionMajor = att.read<int>();
        att = file.getAttribute(FILE_VERSION_MINOR_FIELD_NAME);
        VersionMinor = att.read<int>();
    } catch (h5::Exception &e) {
        if (os)
            (*os) << e.what() << endl;
        return -1;
    }
    return 0;
}

// dump data using H5Easy, create attribute with the description and write description to var_list
template <class T>
int dump(h5::File &file, const std::string &path, const std::vector<T> &data,
         const std::vector<size_t> &dim, const std::string &desc)
{
    if (dim.size() == 1) {
        if (dim[0] == 1) { // scalar
            h5::DataSet dset = h5e::dump(file, path, data[0]);
            dset.createAttribute("description", desc);
            return 0;
        } else { // vector
            h5::DataSet dset = h5e::dump(file, path, data);
            dset.createAttribute("description", desc);
            return 0;
        }
    } else { // multi-dim array - no strings
        h5::DataSet dset = file.createDataSet<T>(path, h5::DataSpace(dim));
        dset.createAttribute("description", desc);
        dset.write_raw(&data[0]);
        return 0;
    }
}

template <typename T>
int load_array(h5::File &file, const std::string &path, ArrayND<T> &A, const size_t &N = 1)
{
    auto dset = file.getDataSet(path);

    auto adim = A.dim();
    auto dsetdim = dset.getDimensions();
    assert(dset.getDimensions() == A.dim());

    dset.read(A.data());

    for (size_t i = 0; i < A.size(); i++)
        A[i] *= N;

    return 0;
}

template <typename T>
int load_array(h5::File &file, const std::string &path, ArrayND<T> &A, ArrayND<T> &dA,
               const size_t &N)
{
    assert(A.size() == dA.size());
    assert(N > 1);
    std::string dpath = path + "_sem";

    int ret = load_array(file, path, A, 1) + load_array(file, dpath, dA, 1);

    if (ret != 0)
        return ret;

    for (size_t i = 0; i < A.size(); i++) {
        dA[i] = N * (A[i] * A[i] + (N - 1) * dA[i] * dA[i]);
        A[i] *= N;
    }

    return 0;
}

int dump_event_stream(h5::File &h5f, const std::string &grp_name, event_stream &es)
{
    // get row, column numbers
    size_t nrows(es.rows()), ncols(es.cols());

    std::string path;
    path = grp_name + "/column_names";
    const auto &s1 = es.event_prototype().columnNames();
    dump(h5f, path, s1, { s1.size() }, "Event data column names");
    path = grp_name + "/column_descriptions";
    const auto &s2 = es.event_prototype().columnDescriptions();
    dump(h5f, path, s2, { s2.size() }, "Event data column descriptions");

    path = grp_name + "/event_data";

    if (nrows == 0) {
        // No data. Create empty dataset and leave
        h5::DataSet dataset = h5f.createDataSet<float>(path, h5::DataSpace(nrows, ncols));
        return 0;
    }

    // mem buffer ~1MB
    size_t buff_rows = std::ceil(1. * (1 << 20) / 4 / ncols);
    buff_rows = std::min(buff_rows, nrows);
    std::vector<float> buff(buff_rows * ncols);

    // Create the dataset.
    // Use compression + chunking
    h5::DataSetCreateProps dscp;
    dscp.add(h5::Deflate(6));
    dscp.add(h5::Chunking({ buff_rows, ncols }));
    h5::DataSet dataset = h5f.createDataSet<float>(path, h5::DataSpace(nrows, ncols), dscp);

    std::vector<size_t> offset{ 0, 0 };
    std::vector<size_t> count{ buff_rows, ncols };

    es.rewind();

    // copy data in chunks
    while (nrows) {
        count[0] = std::min(nrows, buff_rows); // # of rows to copy in this iter
        nrows -= count[0];

        // read from raw file buffer
        es.read(buff.data(), count[0]);

        // write to HDF5 file
        dataset.select(offset, count).write_raw<float>(buff.data());

        // advance offset
        offset[0] += count[0];
    }

    return 0;
}

int load_event_stream(h5::File &h5f, const std::string &grp_name, event_stream &es)
{
    if (!es.is_open())
        return -1;

    // get row, column numbers
    size_t ncols(es.cols());

    std::string path;
    path = grp_name + "/event_data";

    // Get the dataset.
    h5::DataSet dataset = h5f.getDataSet(path);

    std::vector<size_t> dims = dataset.getSpace().getDimensions();
    size_t nrows = dims[0];
    assert(ncols == dims[1]);

    // mem buffer ~1MB
    size_t buff_rows = std::ceil(1. * (1 << 20) / 4 / ncols);
    buff_rows = std::min(buff_rows, nrows);
    std::vector<float> buff(buff_rows * ncols);

    std::vector<size_t> offset{ 0, 0 };
    std::vector<size_t> count{ buff_rows, ncols };

    es.rewind();

    // copy data in chunks
    while (nrows) {
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

// save data from mcinfo, called recursively
int dump(h5::File &h5f, const std::string &path, const mcinfo &i)
{

    std::vector<std::string> s;
    std::vector<double> x, dx;
    std::vector<float> f;
    std::vector<uint64_t> u;
    std::vector<size_t> dim;

    std::string child_path;

    switch (i.type()) {
    case mcinfo::group:
        for (auto &ch : i.children()) {
            dump(h5f, path + '/' + ch.first, ch.second);
        }
        break;
    case mcinfo::string:
        i.get(s, dim);
        dump(h5f, path, s, dim, i.description());
        break;
    case mcinfo::json:
        i.get(s, dim);
        dump(h5f, path, s, dim, i.description());
        break;
    case mcinfo::real64:
        i.get(x, dim);
        dump(h5f, path, x, dim, i.description());
        break;
    case mcinfo::real32:
        i.get(f, dim);
        dump(h5f, path, f, dim, i.description());
        break;
    case mcinfo::uint64:
        i.get(u, dim);
        dump(h5f, path, u, dim, i.description());
        break;
    case mcinfo::tally_score:
        i.get(x, dx, dim);
        dump(h5f, path, x, dim, i.description());
        dump(h5f, path + "_sem", dx, dim, i.description() + " (SEM)");
        break;
    default:
        break;
    }

    return 0;
}

int mcdriver::save(const std::string &h5filename, std::ostream *os)
{
    try {
        h5::File h5f(h5filename, h5::File::Truncate);

        if (writeFileHeader(h5f, os) != 0)
            return -1;

        mcinfo mci(this);
        dump(h5f, "", mci);

        // events
        std::string page = "/events/";
        if (config_.Output.store_pka_events)
            dump_event_stream(h5f, page + "pka", s_->pka_stream());
        if (config_.Output.store_exit_events)
            dump_event_stream(h5f, page + "exit", s_->exit_stream());
        if (config_.Output.store_damage_events)
            dump_event_stream(h5f, page + "damage", s_->damage_stream());

    } catch (h5::Exception &e) {
        if (os)
            (*os) << e.what() << endl;
        return -1;
    }

    return 0;
}

int mcdriver::load(const std::string &h5filename, std::ostream *os)
{
    if (s_) {
        if (os)
            (*os) << "Reset the simulation object before calling load";
        return -1;
    }

    try {
        h5::File h5f(h5filename, h5::File::ReadOnly);

        int vMajor, vMinor;
        if (readFileHeader(h5f, vMajor, vMinor, os) != 0)
            return -1;
        // TODO
        // In future versions check file version

        // load and check simulation mcconfig
        mcconfig opt;
        {
            std::string json = h5e::load<std::string>(h5f, "/run_info/json_config");
            std::stringstream is(json);
            if (opt.parseJSON(is, true, os) != 0)
                return -1;
        }

        // set options in driver. simulation object is created
        init(opt);

        // load run history
        {
            std::string json_str = h5e::load<std::string>(h5f, "/run_info/run_history");
            ojson j = ojson::parse(json_str);
            run_history_ = j.template get<std::vector<mcdriver::run_data>>();
        }

        // Load the rng state
        {
            // get a copy of the current state, as a std::array
            auto rngstate = s_->rngState();
            // load a vector, because std::array is not supported
            std::vector<random_vars::result_type> state_cpy =
                    h5e::load<std::vector<random_vars::result_type>>(h5f, "/run_info/rng_state");
            // copy into std::array
            for (int i = 0; i < state_cpy.size(); ++i)
                rngstate[i] = state_cpy[i];
            // set it in simulation object
            s_->setRngState(rngstate);
        }

        // load the tally data
        size_t Nh = h5e::load<size_t>(h5f, "/run_info/total_ion_count");
        if (Nh) {

            // std tallies
            {
                bool ret = true;
                int k = 1;

                tally &t = s_->getTally();
                tally &dt = s_->getTallyVar();

                while (ret && k < tally::std_tallies) {
                    std::string name("/tally/");
                    name += tally::arrayGroup(k);
                    name += "/";
                    name += tally::arrayName(k);
                    ret = load_array(h5f, name, t.at(k), dt.at(k), Nh) == 0;
                    k++;
                }

                ret = ret && load_array(h5f, "/tally/totals/data", t.at(0), dt.at(0), Nh) == 0;

                if (!ret)
                    return -1;
            }

            // user tallies
            {
                bool ret = true;
                auto &t = s_->getUserTally();
                auto &dt = s_->getUserTallyVar();
                for (int i = 0; i < t.size(); ++i) {
                    std::string name("/user_tally/");
                    name += t[i]->id();
                    name += "/data";
                    ret = load_array(h5f, name, t[i]->data(), dt[i]->data(), Nh) == 0;
                }
            }

            // set the ion count
            s_->setIonCount(Nh);
        }

        // prepare to load events
        uint32_t ev_mask{ 0 };
        if (config_.Output.store_pka_events)
            ev_mask |= static_cast<uint32_t>(Event::CascadeComplete);
        if (config_.Output.store_exit_events)
            ev_mask |= static_cast<uint32_t>(Event::IonExit);
        if (config_.Output.store_damage_events)
            ev_mask |= static_cast<uint32_t>(Event::Vacancy);
        s_->init_streams(ev_mask);

        // load pka events
        if (config_.Output.store_pka_events) {
            load_event_stream(h5f, "/events/pka", s_->pka_stream());
        }

        // load exit events
        if (config_.Output.store_exit_events) {
            load_event_stream(h5f, "/events/exit", s_->exit_stream());
        }

        // load damage events
        if (config_.Output.store_damage_events) {
            load_event_stream(h5f, "/events/damage", s_->damage_stream());
        }

    } catch (h5::Exception &e) {
        if (os)
            (*os) << e.what() << endl;
        return -1;
    }

    return 0;
}
