#include "mcinfo.h"

mcinfo::mcinfo(const mcdriver *d) : driver_(d), type_(mcinfo::group)
{
    // 1. run_info
    (*this)["run_info"] = { driver_, mcinfo::group };
    {
        mcinfo &p = children_.find("run_info")->second;
        p["title"] = { driver_, mcinfo::string, "User supplied simulation title",
                       [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                           s.resize(1);
                           s[0] = i.driver()->config().Output.title;
                           d = { 1 };
                       } };

        p["total_ion_count"] = { driver_, "Total number of simulated ions",
                                 [](const mcinfo &i, std::vector<uint64_t> &s, dim_t &d) {
                                     s.resize(1);
                                     s[0] = i.driver()->getSim()->ion_count();
                                     d = { 1 };
                                 } };

        p["json_config"] = { driver_, mcinfo::string, "JSON formatted simulation options",
                             [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                 s.resize(1);
                                 s[0] = i.driver()->config().toJSON();
                                 d = { 1 };
                             } };

        p["version_info"] = { driver_, mcinfo::group };
        {
            mcinfo &p1 = p.children_.find("version_info")->second;
            p1["name"] = { driver_, mcinfo::string, "code name",
                           [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                               s.resize(1);
                               s[0] = std::string(PROJECT_NAME);
                               d = { 1 };
                           } };
            p1["version"] = { driver_, mcinfo::string, "code version",
                              [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                  s.resize(1);
                                  s[0] = std::string(PROJECT_VERSION);
                                  d = { 1 };
                              } };
            p1["compiler"] = { driver_, mcinfo::string, "compiler",
                               [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                   s.resize(1);
                                   s[0] = std::string(COMPILER_ID);
                                   d = { 1 };
                               } };
            p1["compiler_version"] = { driver_, mcinfo::string, "compiler_version",
                                       [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                           s.resize(1);
                                           s[0] = std::string(COMPILER_VERSION);
                                           d = { 1 };
                                       } };
            p1["build_system"] = { driver_, mcinfo::string, "build system",
                                   [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                       s.resize(1);
                                       s[0] = std::string(SYSTEM_ID);
                                       d = { 1 };
                                   } };
            p1["build_time"] = { driver_, mcinfo::string, "build time",
                                 [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                     s.resize(1);
                                     s[0] = std::string(BUILD_TIME);
                                     d = { 1 };
                                 } };
        }
        p["run_history"] = { driver_, mcinfo::json, "run history",
                             [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                 s.resize(1);
                                 ojson j = i.driver()->run_history();
                                 std::ostringstream os;
                                 os << j.dump(4) << std::endl;
                                 s[0] = os.str();
                                 d = { 1 };
                             } };
        p["rng_state"] = { driver_, "random generator state",
                           [](const mcinfo &ii, std::vector<uint64_t> &s, dim_t &d) {
                               const auto &rngstate = ii.driver()->getSim()->rngState();
                               // make an element copy, because std::array is not supported
                               s.resize(rngstate.size());
                               for (int i = 0; i < rngstate.size(); ++i)
                                   s[i] = rngstate[i];
                               d.resize(1);
                               d[0] = rngstate.size();
                           } };
    }

    // 2. target
    (*this)["target"] = { driver_, mcinfo::group };
    {
        mcinfo &p = children_.find("target")->second;
        // target/grid
        p["grid"] = { driver_, mcinfo::group };
        {
            mcinfo &p1 = p.children_.find("grid")->second;
            p1["X"] = { driver_, "x-axis grid",
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            s = i.driver_->getSim()->getTarget().grid().x();
                            d = { s.size() };
                        } };
            p1["Y"] = { driver_, "y-axis grid",
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            s = i.driver_->getSim()->getTarget().grid().y();
                            d = { s.size() };
                        } };
            p1["Z"] = { driver_, "z-axis grid",
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            s = i.driver_->getSim()->getTarget().grid().z();
                            d = { s.size() };
                        } };
            p1["cell_xyz"] = { driver_, "Cell center coordinates",
                               [](const mcinfo &i, std::vector<float> &buff, dim_t &d) {
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
                               } };
        }

        // target/materials
        p["materials"] = { driver_, mcinfo::group };
        {
            mcinfo &p1 = p.children_.find("materials")->second;
            p1["name"] = { driver_, mcinfo::string, "name of material",
                           [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                               const auto &mat = i.driver()->getSim()->getTarget().materials();
                               s.resize(mat.size());
                               for (int k = 0; k < mat.size(); ++k)
                                   s[k] = mat[k]->name();
                               d = { s.size() };
                           } };
            p1["atomic_density"] = { driver_, "atomic density [at/nm^3]",
                                     [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                         const auto &mat =
                                                 i.driver()->getSim()->getTarget().materials();
                                         s.resize(mat.size());
                                         for (int k = 0; k < mat.size(); ++k)
                                             s[k] = mat[k]->atomicDensity();
                                         d = { s.size() };
                                     } };
            p1["mass_density"] = { driver_, "mass density [g/cm^3]",
                                   [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                       const auto &mat =
                                               i.driver()->getSim()->getTarget().materials();
                                       s.resize(mat.size());
                                       for (int k = 0; k < mat.size(); ++k)
                                           s[k] = mat[k]->massDensity();
                                       d = { s.size() };
                                   } };
            p1["atomic_radius"] = { driver_, "atomic radius [nm]",
                                    [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                        const auto &mat =
                                                i.driver()->getSim()->getTarget().materials();
                                        s.resize(mat.size());
                                        for (int k = 0; k < mat.size(); ++k)
                                            s[k] = mat[k]->atomicRadius();
                                        d = { s.size() };
                                    } };
        }

        // target/atoms
        p["atoms"] = { driver_, mcinfo::group };
        {
            mcinfo &p1 = p.children_.find("atoms")->second;
            p1["label"] = { driver_, mcinfo::string,
                            "label = [Atom (Chemical symbol)] in [Material]",
                            [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                s = i.driver()->getSim()->getTarget().atom_labels();
                                d = { s.size() };
                            } };
            p1["symbol"] = { driver_, mcinfo::string, "Chemical symbol",
                             [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                 const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                                 s.resize(atoms.size());
                                 for (int k = 0; k < atoms.size(); ++k)
                                     s[k] = atoms[k]->symbol();
                                 d = { s.size() };
                             } };
            p1["Z"] = { driver_, "Atomic number",
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                            s.resize(atoms.size());
                            for (int k = 0; k < atoms.size(); ++k)
                                s[k] = atoms[k]->Z();
                            d = { s.size() };
                        } };
            p1["M"] = { driver_, "Atomic mass",
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                            s.resize(atoms.size());
                            for (int k = 0; k < atoms.size(); ++k)
                                s[k] = atoms[k]->M();
                            d = { s.size() };
                        } };
            p1["Ed"] = { driver_, "Displacement energy [eV]",
                         [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                             const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                             s.resize(atoms.size());
                             for (int k = 0; k < atoms.size(); ++k)
                                 s[k] = atoms[k]->Ed();
                             d = { s.size() };
                         } };
            p1["El"] = { driver_, "Lattice Binding energy [eV]",
                         [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                             const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                             s.resize(atoms.size());
                             for (int k = 0; k < atoms.size(); ++k)
                                 s[k] = atoms[k]->El();
                             d = { s.size() };
                         } };
            p1["Es"] = { driver_, "Surface binding energy [eV]",
                         [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                             const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                             s.resize(atoms.size());
                             for (int k = 0; k < atoms.size(); ++k)
                                 s[k] = atoms[k]->Es();
                             d = { s.size() };
                         } };
            p1["Er"] = { driver_, "Replacement energy [eV]",
                         [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                             const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                             s.resize(atoms.size());
                             for (int k = 0; k < atoms.size(); ++k)
                                 s[k] = atoms[k]->Er();
                             d = { s.size() };
                         } };
            p1["Rc"] = { driver_, "Recombination radius [nm]",
                         [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                             const auto &atoms = i.driver()->getSim()->getTarget().atoms();
                             s.resize(atoms.size());
                             for (int k = 0; k < atoms.size(); ++k)
                                 s[k] = atoms[k]->Rc();
                             d = { s.size() };
                         } };
        }

        if (driver()->config().Output.store_dedx) {
            // target/dedx

            p["dedx"] = { driver_, mcinfo::group };
            {
                mcinfo &p1 = p.children_.find("dedx")->second;
                p1["erg"] = { driver_, "dEdx table energy grid [eV]",
                              [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                  s.resize(dedx_erange::size());
                                  for (dedx_iterator k; k < k.end(); k++)
                                      s[k] = *k;
                                  d = { s.size() };
                              } };
                p1["eloss"] = {
                    driver_, "electronic dEdx values [eV/nm], array [ions x materials x energy]",
                    [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                        ArrayND<dedx_interp *> D = i.driver()->getSim()->get_dedx_calc().dedx();
                        d.resize(3);
                        d[0] = D.dim()[0];
                        d[1] = D.dim()[1];
                        d[2] = dedx_erange::size();
                        size_t n = d[0] * d[1] * d[2];
                        s.resize(n);
                        for (int i = 0; i < d[0]; ++i)
                            for (int j = 0; j < d[1]; ++j) {
                                if (D(i, j)) {
                                    memcpy(s.data() + (i * d[1] + j) * d[2], D(i, j)->data().data(),
                                           dedx_erange::size() * sizeof(float));
                                }
                            }
                    }
                };
                p1["strag"] = { driver_,
                                "electronic straggling [eV], array [ions x materials x energy]",
                                [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                    ArrayND<straggling_interp *> D =
                                            i.driver()->getSim()->get_dedx_calc().de_strag();
                                    d.resize(3);
                                    d[0] = D.dim()[0];
                                    d[1] = D.dim()[1];
                                    d[2] = dedx_erange::size();
                                    size_t n = d[0] * d[1] * d[2];
                                    s.resize(n, 0.f);
                                    for (int i = 0; i < d[0]; ++i)
                                        for (int j = 0; j < d[1]; ++j) {
                                            if (D(i, j)) {
                                                memcpy(s.data() + (i * d[1] + j) * d[2],
                                                       D(i, j)->data().data(),
                                                       dedx_erange::size() * sizeof(float));
                                            }
                                        }
                                } };
            }

            // target/flight_path
            p["flight_path"] = { driver_, mcinfo::group };
            {
                mcinfo &p1 = p.children_.find("flight_path")->second;
                p1["erg"] = { driver_, "flight path table energy grid [eV]",
                              [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                  s.resize(flight_path_calc::fp_tbl_erange::size());
                                  for (flight_path_calc::fp_tbl_iterator k; k < k.end(); k++)
                                      s[k] = *k;
                                  d = { s.size() };
                              } };

                p1["mfp"] = { driver_, "mean free path [nm], array [ions x materials x energy]",
                              [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                  ArrayNDf D = i.driver()->getSim()->get_fp_calc().mfp();
                                  d = D.dim();
                                  size_t n = D.size();
                                  s.resize(n);
                                  memcpy(&s[0], D.data(), n * sizeof(float));
                              } };

                p1["ipmax"] = { driver_,
                                "max impact parameter [nm], array [ions x materials x energy]",
                                [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                    ArrayNDf D = i.driver()->getSim()->get_fp_calc().ipmax();
                                    d = D.dim();
                                    size_t n = D.size();
                                    s.resize(n);
                                    memcpy(&s[0], D.data(), n * sizeof(float));
                                } };

                p1["fpmax"] = { driver_, "max flight path [nm], array [ions x materials x energy]",
                                [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                    ArrayNDf D = i.driver()->getSim()->get_fp_calc().fpmax();
                                    d = D.dim();
                                    size_t n = D.size();
                                    s.resize(n);
                                    memcpy(&s[0], D.data(), n * sizeof(float));
                                } };
            }
        }
    }

    // 3. ion beam
    (*this)["ion_beam"] = { driver_, mcinfo::group };
    {
        mcinfo &p = children_.find("ion_beam")->second;
        p["E0"] = {
            driver_, "Mean energy [eV]",
            [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                s.resize(1);
                s[0] = i.driver_->getSim()->getSource().getParameters().energy_distribution.center;
                d = { 1 };
            }
        };

        p["Z"] = { driver_, "Atomic number", [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                      s.resize(1);
                      s[0] = i.driver_->getSim()->getSource().getParameters().ion.atomic_number;
                      d = { 1 };
                  } };

        p["M"] = { driver_, "Atomic mass [emu]",
                   [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                       s.resize(1);
                       s[0] = i.driver_->getSim()->getSource().getParameters().ion.atomic_mass;
                       d = { 1 };
                   } };

        p["dir0"] = { driver_, "Mean direction",
                      [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                          s.resize(3);
                          const auto &p = i.driver_->getSim()->getSource().getParameters();
                          s[0] = p.angular_distribution.center.x();
                          s[1] = p.angular_distribution.center.y();
                          s[2] = p.angular_distribution.center.z();
                          d = { 3 };
                      } };

        p["x0"] = { driver_, "Mean position", [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                       s.resize(3);
                       const auto &p = i.driver_->getSim()->getSource().getParameters();
                       s[0] = p.spatial_distribution.center.x();
                       s[1] = p.spatial_distribution.center.y();
                       s[2] = p.spatial_distribution.center.z();
                       d = { 3 };
                   } };
    }

    // 4. tally
    (*this)["tally"] = { driver_, mcinfo::group };
    {
        mcinfo &p = children_.find("tally")->second;

        std::map<std::string, std::map<std::string, int>> tmap;
        int k = 1;
        while (k < tally::std_tallies) {
            tmap[tally::arrayGroup(k)][tally::arrayName(k)] = k;
            k++;
        }

        for (const auto &tgroup : tmap) {
            p[tgroup.first] = { driver_, mcinfo::group };
            mcinfo &p1 = p.children_.find(tgroup.first)->second;
            for (const auto &tname : tgroup.second) {
                int it = tname.second;
                p1[tname.first] = { driver_, tally::arrayDescription(it), it };
            }
        }

        p["totals"] = { driver_, mcinfo::group };
        {
            mcinfo &p1 = p.children_.find("totals")->second;
            p1["data"] = { driver_, "tally totals per atom", 0 };
            p1["column_names"] = { driver_, mcinfo::string, "names of totals",
                                   [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                       s = i.driver_->getSim()->getTally().arrayNames();
                                       d = { s.size() };
                                   } };
        }
    }

    // 5. user_tally
    (*this)["user_tally"] = { driver_, mcinfo::group };
    {
        mcinfo &p = children_.find("user_tally")->second;

        const auto &utv = driver_->getSim()->getUserTally();

        for (int iut = 0; iut < utv.size(); ++iut) { // only if a user tally has been defined

            const user_tally *ut = utv[iut];

            p[ut->id()] = { driver_, mcinfo::group };
            {
                mcinfo &p1 = p.children_.find(ut->id())->second;

                p1["data"] = { driver_, "bin data per ion", -1, iut };

                p1["decription"] = { driver_, mcinfo::string, "Tally description",
                                     [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                         const user_tally *ut =
                                                 i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                                         s.resize(1);
                                         s[0] = ut->description();
                                         d = { 1 };
                                     },
                                     iut };

                p1["coordinate_system"] = { driver_, mcinfo::group };
                {
                    mcinfo &p2 = p1.children_.find("coordinate_system")->second;

                    // 3 x 3D vectors that define the coordinate system of the user_tally
                    p2["zaxis"] = { driver_, "Vector parallel to the z-axis direction",
                                    [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                        const user_tally *ut =
                                                i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                                        const vector3 &v = ut->cs().zaxis;
                                        s = { v.x(), v.y(), v.z() };
                                        d = { 3 };
                                    },
                                    iut };

                    p2["xzvector"] = {
                        driver_, "Vector on the xz-plane",
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            const user_tally *ut =
                                    i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                            const vector3 &v = ut->cs().xzvector;
                            s = { v.x(), v.y(), v.z() };
                            d = { 3 };
                        },
                        iut
                    };

                    p2["origin"] = { driver_, "Coordinate system origin",
                                     [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                                         const user_tally *ut =
                                                 i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                                         const vector3 &v = ut->cs().origin;
                                         s = { v.x(), v.y(), v.z() };
                                         d = { 3 };
                                     },
                                     iut };
                }

                // Event
                p1["event"] = { driver_, mcinfo::string, "Tally event",
                                [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                    const user_tally *ut =
                                            i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                                    s.resize(1);
                                    s[0] = event_name(ut->event());
                                    d = { 1 };
                                },
                                iut };

                p1["event_decription"] = {
                    driver_, mcinfo::string, "Tally event description",
                    [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        const user_tally *ut = i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                        s.resize(1);
                        s[0] = event_description(ut->event());
                        d = { 1 };
                    },
                    iut
                };

                // Bin names
                p1["bin_names"] = { driver_, mcinfo::string, "Bin names",
                                    [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                                        const user_tally *ut =
                                                i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                                        s = ut->bin_names();
                                        d = { s.size() };
                                    },
                                    iut };
                p1["bin_descriptions"] = {
                    driver_, mcinfo::string, "Bin descriptions",
                    [](const mcinfo &i, std::vector<std::string> &s, dim_t &d) {
                        const user_tally *ut = i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                        s = ut->bin_descriptions();
                        d = { s.size() };
                    },
                    iut
                };

                // Save bins
                p1["bins"] = { driver_, mcinfo::group, "Bin edges" };
                std::vector<std::string> names = ut->bin_names();
                std::vector<std::string> desc = ut->bin_descriptions();
                mcinfo &p2 = p1.children_.find("bins")->second;
                for (int j = 0; j < names.size(); ++j) {
                    std::ostringstream os;
                    os << "Bin edges of dim " << j << " : [" << names[j] << "]";

                    p2[std::to_string(j)] = {
                        driver_, os.str(),
                        [](const mcinfo &i, std::vector<float> &s, dim_t &d) {
                            const user_tally *ut =
                                    i.driver_->getSim()->getUserTally()[i.extra_idx1_];
                            s = ut->bin_edges(i.extra_idx0_);
                            d = { s.size() };
                        },
                        iut
                    };
                    p2.children_.find(std::to_string(j))->second.extra_idx0_ = j;
                }
            }
        }
    }
}

bool mcinfo::get(std::vector<double> &s, std::vector<double> &ds, dim_t &dim) const
{
    if (driver_->getSim() == nullptr)
        return false;

    ArrayNDd A;
    ArrayNDd dA;
    if (extra_idx0_ >= 0) {
        A = driver_->getSim()->getTally().at(extra_idx0_);
        dA = driver_->getSim()->getTallyVar().at(extra_idx0_);
    } else {
        const user_tally *ut = driver_->getSim()->getUserTally()[extra_idx1_];
        const user_tally *dut = driver_->getSim()->getUserTallyVar()[extra_idx1_];
        if (!ut)
            return false;
        A = ut->data();
        dA = dut->data();
    }

    const size_t &N = driver_->getSim()->ion_count();
    s.resize(A.size(), 0.0);
    ds.resize(A.size(), 0.0);
    for (int i = 0; i < A.size(); i++) {
        s[i] = A[i] / N;
        // error in the mean
        ds[i] = (N > 1) ? std::sqrt((dA[i] / N - s[i] * s[i]) / (N - 1)) : 0;
    }
    dim = A.dim();
    return true;
}
