#include "mcinfo.h"

#include "json_defs_p.h"

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcdriver::run_data, start_time, end_time, ips, cpu_time,
                                          nthreads, run_ion_count, total_ion_count)

mcinfo::mcinfo(const mcdriver &d) : driver_(&d), type_(mcinfo::group)
{
    // 1. run_info
    mcinfo *p = add("run_info", mcinfo::group);
    {
        mcinfo *p1 = p->add("title", mcinfo::string, "User supplied simulation title");
        p1->string_getter_.get_ = [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
            s.resize(1);
            s[0] = i.driver()->config().Output.title;
            d = { 1 };
        };
        p1 = p->add("total_ion_count", mcinfo::uint64, "Total number of simulated ions");
        p1->uint64_getter_.get_ = [](mcinfo &i, std::vector<uint64_t> &s, dim_t &d) {
            s.resize(1);
            s[0] = i.driver()->getSim()->ion_count();
            d = { 1 };
        };
        p1 = p->add("json_config", mcinfo::json, "JSON formatted simulation options");
        p1->string_getter_.get_ = [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
            s.resize(1);
            s[0] = i.driver()->config().toJSON();
            d = { 1 };
        };
        p1 = p->add("version_info", mcinfo::group);
        {
            p1->add("name", mcinfo::string, "code name")->string_getter_.get_ =
                    [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        s.resize(1);
                        s[0] = std::string(PROJECT_NAME);
                        d = { 1 };
                    };
            p1->add("version", mcinfo::string, "code version")->string_getter_.get_ =
                    [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        s.resize(1);
                        s[0] = std::string(PROJECT_VERSION);
                        d = { 1 };
                    };
            p1->add("compiler", mcinfo::string, "compiler")->string_getter_.get_ =
                    [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        s.resize(1);
                        s[0] = std::string(COMPILER_ID);
                        d = { 1 };
                    };
            p1->add("compiler_version", mcinfo::string, "compiler version")->string_getter_.get_ =
                    [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        s.resize(1);
                        s[0] = std::string(COMPILER_VERSION);
                        d = { 1 };
                    };
            p1->add("build_system", mcinfo::string, "build system")->string_getter_.get_ =
                    [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        s.resize(1);
                        s[0] = std::string(SYSTEM_ID);
                        d = { 1 };
                    };
            p1->add("build_time", mcinfo::string, "build time")->string_getter_.get_ =
                    [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        s.resize(1);
                        s[0] = std::string(BUILD_TIME);
                        d = { 1 };
                    };
        }
        p1 = p->add("run_history", mcinfo::json, "run history");
        p1->string_getter_.get_ = [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
            s.resize(1);
            ojson j = i.driver()->run_history();
            std::ostringstream os;
            os << j.dump(4) << std::endl;
            s[0] = os.str();
            d = { 1 };
        };
        p1 = p->add("rng_state", mcinfo::uint64, "random generator state");
        p1->uint64_getter_.get_ = [](mcinfo &ii, std::vector<uint64_t> &s, dim_t &d) {
            const auto &rngstate = ii.driver()->getSim()->rngState();
            // make an element copy, because std::array is not supported
            s.resize(rngstate.size());
            for (int i = 0; i < rngstate.size(); ++i)
                s[i] = rngstate[i];
            d.resize(1);
            d[0] = rngstate.size();
        };
    }

    // 2. target
    p = add("target", mcinfo::group);
    {
        // target/grid
        mcinfo *p1 = p->add("grid", mcinfo::group);
        {
            mcinfo *p2 = p1->add("X", mcinfo::real32, "x-axis grid");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                s = i.driver_->getSim()->getTarget().grid().x();
                d = { s.size() };
            };
            p2 = p1->add("Y", mcinfo::real32, "y-axis grid");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                s = i.driver_->getSim()->getTarget().grid().y();
                d = { s.size() };
            };
            p2 = p1->add("Z", mcinfo::real32, "z-axis grid");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                s = i.driver_->getSim()->getTarget().grid().z();
                d = { s.size() };
            };
            p2 = p1->add("cell_xyz", mcinfo::real32, "Cell center coordinates");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &buff, dim_t &d) {
                const auto &grid = i.driver_->getSim()->getTarget().grid();
                int rows = grid.x().size() - 1;
                int cols = grid.y().size() - 1;
                int layers = grid.z().size() - 1;
                buff.resize(3 * grid.ncells());

                for (int i = 0; i < rows; i++)
                    for (int j = 0; j < cols; j++)
                        for (int k = 0; k < layers; k++) {
                            int l = (i * cols + j) * layers + k;
                            int m = l;
                            buff[m] = 0.5f * (grid.x()[i] + grid.x()[i + 1]);
                            m += grid.ncells();
                            buff[m] = 0.5f * (grid.y()[j] + grid.y()[j + 1]);
                            m += grid.ncells();
                            buff[m] = 0.5f * (grid.z()[k] + grid.z()[k + 1]);
                        }
                d = { 3, (size_t)grid.ncells() };
            };
        }

        // target/materials
        p1 = p->add("materials", mcinfo::group);
        {
            mcinfo *p2 = p1->add("name", mcinfo::string, "name of material");
            p2->string_getter_.get_ = [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                const auto &mat = i.driver()->getSim()->getTarget().materials();
                s.resize(mat.size());
                for (int k = 0; k < mat.size(); ++k)
                    s[k] = mat[k]->name();
                d = { s.size() };
            };
            p2 = p1->add("atomic_density", mcinfo::real32, "atomic density [at/nm^3]");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                const auto &mat = i.driver()->getSim()->getTarget().materials();
                s.resize(mat.size());
                for (int k = 0; k < mat.size(); ++k)
                    s[k] = mat[k]->atomicDensity();
                d = { s.size() };
            };
            p2 = p1->add("mass_density", mcinfo::real32, "mass density [g/cm^3]");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                const auto &mat = i.driver()->getSim()->getTarget().materials();
                s.resize(mat.size());
                for (int k = 0; k < mat.size(); ++k)
                    s[k] = mat[k]->massDensity();
                d = { s.size() };
            };
            p2 = p1->add("atomic_radius", mcinfo::real32, "atomic radius [nm]");
            p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                const auto &mat = i.driver()->getSim()->getTarget().materials();
                s.resize(mat.size());
                for (int k = 0; k < mat.size(); ++k)
                    s[k] = mat[k]->atomicRadius();
                d = { s.size() };
            };
        }

        if (driver()->config().Output.store_dedx) {
            // target/dedx
            p1 = p->add("dedx", mcinfo::group);
            {
                mcinfo *p2 = p1->add("erg", mcinfo::real32, "dEdx table energy grid [eV]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    s.resize(dedx_index::size);
                    for (dedx_index k; k < k.end(); k++)
                        s[k] = *k;
                    d = { s.size() };
                };

                p2 = p1->add("eloss", mcinfo::real32,
                             "electronic dEdx values [eV/nm], array [ions x materials x "
                             "energy]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    ArrayND<dedx_interp *> D = i.driver()->getSim()->get_dedx_calc().dedx();
                    d.resize(3);
                    d[0] = D.dim()[0];
                    d[1] = D.dim()[1];
                    d[2] = dedx_index::size;
                    size_t n = d[0] * d[1] * d[2];
                    s.resize(n);
                    for (int i = 0; i < d[0]; ++i)
                        for (int j = 0; j < d[1]; ++j)
                            memcpy(s.data() + (i * d[1] + j) * d[2], D(i, j)->data(),
                                   dedx_index::size * sizeof(float));
                };

                p2 = p1->add("strag", mcinfo::real32,
                             "electronic straggling [eV], array [ions x materials x "
                             "energy]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    ArrayND<straggling_interp *> D =
                            i.driver()->getSim()->get_dedx_calc().de_strag();
                    d.resize(3);
                    d[0] = D.dim()[0];
                    d[1] = D.dim()[1];
                    d[2] = dedx_index::size;
                    size_t n = d[0] * d[1] * d[2];
                    s.resize(n);
                    for (int i = 0; i < d[0]; ++i)
                        for (int j = 0; j < d[1]; ++j)
                            memcpy(s.data() + (i * d[1] + j) * d[2], D(i, j)->data(),
                                   dedx_index::size * sizeof(float));
                };
            }

            // target/flight_path
            p1 = p->add("flight_path", mcinfo::group);
            {
                mcinfo *p2 = p1->add("erg", mcinfo::real32, "flight path table energy grid [eV]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    s.resize(dedx_index::size);
                    for (dedx_index k; k < k.end(); k++)
                        s[k] = *k;
                    d = { s.size() };
                };

                p2 = p1->add("mfp", mcinfo::real32,
                             "mean free path [nm], array [atoms x materials x energy]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    ArrayNDf D = i.driver()->getSim()->get_fp_calc().mfp();
                    d = D.dim();
                    size_t n = D.size();
                    s.resize(n);
                    memcpy(&s[0], D.data(), n * sizeof(float));
                };

                p2 = p1->add("ipmax", mcinfo::real32,
                             "max impact parameter [nm], array [atoms x materials x energy]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    ArrayNDf D = i.driver()->getSim()->get_fp_calc().ipmax();
                    d = D.dim();
                    size_t n = D.size();
                    s.resize(n);
                    memcpy(&s[0], D.data(), n * sizeof(float));
                };

                p2 = p1->add("fpmax", mcinfo::real32,
                             "max flight path [nm], array [atoms x materials x energy]");
                p2->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
                    ArrayNDf D = i.driver()->getSim()->get_fp_calc().fpmax();
                    d = D.dim();
                    size_t n = D.size();
                    s.resize(n);
                    memcpy(&s[0], D.data(), n * sizeof(float));
                };
            }
        }
    }

    // 3. ion beam
    p = add("ion_beam", mcinfo::group);
    {
        mcinfo *p1 = p->add("E0", mcinfo::real32, "Mean energy [eV]");
        p1->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
            s.resize(1);
            s[0] = i.driver_->getSim()->getSource().getParameters().energy_distribution.center;
            d = { 1 };
        };

        p1 = p->add("Z", mcinfo::real32, "Atomic number");
        p1->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
            s.resize(1);
            s[0] = i.driver_->getSim()->getSource().getParameters().ion.atomic_number;
            d = { 1 };
        };

        p1 = p->add("M", mcinfo::real32, "Atomic mass [emu]");
        p1->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
            s.resize(1);
            s[0] = i.driver_->getSim()->getSource().getParameters().ion.atomic_mass;
            d = { 1 };
        };

        p1 = p->add("dir0", mcinfo::real32, "Mean direction");
        p1->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
            s.resize(3);
            s[0] = i.driver_->getSim()->getSource().getParameters().angular_distribution.center.x();
            s[1] = i.driver_->getSim()->getSource().getParameters().angular_distribution.center.y();
            s[2] = i.driver_->getSim()->getSource().getParameters().angular_distribution.center.z();
            d = { 3 };
        };

        p1 = p->add("x0", mcinfo::real32, "Mean position");
        p1->real32_getter_.get_ = [](mcinfo &i, std::vector<float> &s, dim_t &d) {
            s.resize(3);
            s[0] = i.driver_->getSim()->getSource().getParameters().spatial_distribution.center.x();
            s[1] = i.driver_->getSim()->getSource().getParameters().spatial_distribution.center.y();
            s[2] = i.driver_->getSim()->getSource().getParameters().spatial_distribution.center.z();
            d = { 3 };
        };
    }

    // 4. tally
    p = add("tally", mcinfo::group);
    {
        std::map<std::string, std::map<std::string, int>> tmap;
        int k = 1;
        while (k < tally::std_tallies) {
            tmap[tally::arrayGroup(k)][tally::arrayName(k)] = k;
            k++;
        }

        for (const auto &tgroup : tmap) {
            mcinfo *p1 = p->add(tgroup.first, mcinfo::group);
            for (const auto &tname : tgroup.second) {
                int it = tname.second;
                mcinfo *p2 = p1->add(tname.first, mcinfo::tally_score, tally::arrayDescription(it));
                p2->tally_idx_ = it;
            }
        }

        mcinfo *p1 = p->add("totals", mcinfo::group);
        {
            mcinfo *p2 = p1->add("data", mcinfo::tally_score, "tally totals per atom");
            p2->tally_idx_ = 0;

            p2 = p1->add("data", mcinfo::string, "names of totals");
            p2->string_getter_.get_ = [](mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                s = i.driver_->getSim()->getTally().arrayNames();
                d = { s.size() };
            };
        }
    }
}

bool mcinfo::get(std::vector<double> &s, std::vector<double> &ds)
{
    if (driver_->getSim() == nullptr)
        return false;
    ArrayNDd A = driver_->getSim()->getTally().at(tally_idx_);
    ArrayNDd dA = driver_->getSim()->getTallyVar().at(tally_idx_);
    const size_t &N = driver_->getSim()->ion_count();
    s.resize(A.size(), 0.0);
    ds.resize(A.size(), 0.0);
    for (int i = 0; i < A.size(); i++) {
        s[i] = A[i] / N;
        // error in the mean
        ds[i] = (N > 1) ? std::sqrt((dA[i] / N - s[i] * s[i]) / (N - 1)) : 0;
    }
    dim_ = A.dim();
    return true;
}

mcinfo *mcinfo::add(std::string key, info_t t, const std::string &dsc)
{
    mcinfo *i = new mcinfo(t, dsc);
    i->parent_ = this;
    i->driver_ = driver_;
    children_[key] = i;
    return i;
}

void mcinfo::destroy()
{
    for (auto p : children_) {
        if (p.second->type_ == group)
            p.second->destroy();
        delete p.second;
    }
    children_.clear();
}
