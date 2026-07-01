#include "mcinfo.h"

#include "json_defs_p.h"

mcinfo::mcinfo(std::shared_ptr<mcdriver> d) : mcinfo_node({ }, nullptr), driver_(d)
{
    // 1. run_info
    {
        mcinfo &run_info = add_group("run_info", "Run information");
        run_info.add_data(
                "title", "User supplied simulation title",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                    s[0] = i.parent()->driver()->config().Output.title;
                });
        run_info.add_data(
                "total_ion_count", "Total number of simulated ions",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<uint64_t> &s) {
                    s[0] = i.parent()->driver()->getSim()->ion_count();
                });
        run_info.add_data(
                "json_config", "JSON formatted simulation options",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                    s[0] = i.parent()->driver()->config().toJSON();
                },
                true);
        mcinfo &version_info = run_info.add_group("version_info", "opentrim version & build info");
        {
            version_info.add_data(
                    "name", "Executable name",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().project_name;
                    });
            version_info.add_data(
                    "version", "Executable version",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().version;
                    });
            version_info.add_data(
                    "git_tag", "git describe --tags output",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().git_tag;
                    });
            version_info.add_data(
                    "compiler", "C++ compiler",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().compiler_id;
                    });
            version_info.add_data(
                    "compiler_version", "C++ compiler_version",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().compiler_version;
                    });
            version_info.add_data(
                    "build_system", "System used for building the code",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().system_id;
                    });
            version_info.add_data(
                    "build_time", "Build timestamp",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s[0] = mcdriver::version_info().build_time;
                    });
        }
        run_info.add_data(
                "run_history", "JSON formattet run history",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                    ojson j = i.parent()->driver()->run_history();
                    std::ostringstream os;
                    os << j.dump(4) << std::endl;
                    s[0] = os.str();
                },
                true);
        run_info.add_data(
                "rng_state", "random generator state",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                    d = { i.parent()->driver()->getSim()->rngState().size() };
                },
                [](const mcinfo_data_node &i, std::vector<uint64_t> &s) {
                    const auto &rngstate = i.parent()->driver()->getSim()->rngState();
                    // make an element copy, because std::array is not supported
                    for (size_t i = 0; i < rngstate.size(); ++i)
                        s[i] = rngstate[i];
                });
    }

    // 2. target
    {
        mcinfo &target = add_group("target", "Target information");
        {
            mcinfo &grid = target.add_group("grid", "Simulation spatial grid");
            grid.add_data(
                    "X", "x-axis grid",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().grid().x().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        s = i.parent()->driver()->getSim()->getTarget().grid().x();
                    });
            grid.add_data(
                    "Y", "y-axis grid",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().grid().y().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        s = i.parent()->driver()->getSim()->getTarget().grid().y();
                    });
            grid.add_data(
                    "Z", "z-axis grid",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().grid().z().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        s = i.parent()->driver()->getSim()->getTarget().grid().z();
                    });
            grid.add_data(
                    "cell_xyz", "Cell center coordinates",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { 3,
                              (size_t)i.parent()->driver()->getSim()->getTarget().grid().ncells() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        s = i.parent()->driver()->getSim()->getTarget().grid().z();
                        const auto &grid = i.parent()->driver()->getSim()->getTarget().grid();
                        int rows = grid.x().size() - 1;
                        int cols = grid.y().size() - 1;
                        int layers = grid.z().size() - 1;

                        for (int i = 0; i < rows; i++)
                            for (int j = 0; j < cols; j++)
                                for (int k = 0; k < layers; k++) {
                                    int l = (i * cols + j) * layers + k;
                                    int m = l;
                                    s[m] = 0.5f * (grid.x()[i] + grid.x()[i + 1]);
                                    m += grid.ncells();
                                    s[m] = 0.5f * (grid.y()[j] + grid.y()[j + 1]);
                                    m += grid.ncells();
                                    s[m] = 0.5f * (grid.z()[k] + grid.z()[k + 1]);
                                }
                    });
        }
        {
            mcinfo &materials = target.add_group("materials", "Materials of the simulation");
            materials.add_data(
                    "name", "Material name",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().materials().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const auto &mat = i.parent()->driver()->getSim()->getTarget().materials();
                        for (size_t k = 0; k < mat.size(); ++k)
                            s[k] = mat[k]->name();
                    });
            materials.add_data(
                    "atomic_density", "Material atomic density [at/nm^3]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().materials().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &mat = i.parent()->driver()->getSim()->getTarget().materials();
                        for (size_t k = 0; k < mat.size(); ++k)
                            s[k] = mat[k]->atomicDensity();
                    });
            materials.add_data(
                    "mass_density", "Material mass density [g/cm^3]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().materials().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &mat = i.parent()->driver()->getSim()->getTarget().materials();
                        for (size_t k = 0; k < mat.size(); ++k)
                            s[k] = mat[k]->massDensity();
                    });
            materials.add_data(
                    "atomic_radius", "Material atomic radius [nm]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().materials().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &mat = i.parent()->driver()->getSim()->getTarget().materials();
                        for (size_t k = 0; k < mat.size(); ++k)
                            s[k] = mat[k]->atomicRadius();
                    });
        }
        {
            mcinfo &atoms = target.add_group("atoms", "Atoms of the simulation");
            atoms.add_data(
                    "label", "Atom label = [Chemical symbol] in [Material]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        s = i.parent()->driver()->getSim()->getTarget().atom_labels();
                    });
            atoms.add_data(
                    "symbol", "Chemical symbol",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->symbol();
                    });
            atoms.add_data(
                    "Z", "Atomic number",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->Z();
                    });
            atoms.add_data(
                    "M", "Atomic mass",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->M();
                    });
            atoms.add_data(
                    "Ed", "Displacement energy [eV]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->Ed();
                    });
            atoms.add_data(
                    "El", "Lattice binding energy [eV]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->El();
                    });
            atoms.add_data(
                    "Es", "Surface binding energy [eV]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->Es();
                    });
            atoms.add_data(
                    "Er", "Replacement energy [eV]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->Er();
                    });
            atoms.add_data(
                    "Rc", "Recombination radius [nm]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { i.parent()->driver()->getSim()->getTarget().atoms().size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        const auto &atoms = i.parent()->driver()->getSim()->getTarget().atoms();
                        for (size_t k = 0; k < atoms.size(); ++k)
                            s[k] = atoms[k]->Rc();
                    });
        }
        if (driver()->config().Output.store_dedx) {
            // target/dedx
            mcinfo &dedx = target.add_group("dedx", "Stopping & Straggling data");
            dedx.add_data(
                    "erg", "dEdx table energy grid [eV]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { dedx_erange::size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        for (dedx_iterator k; k < k.end(); k++)
                            s[k] = *k;
                    });
            dedx.add_data(
                    "eloss", "electronic dEdx values [eV/nm], array [ions x materials x energy]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        auto &dim = i.parent()->driver()->getSim()->get_dedx_calc().dedx().dim();
                        d = { dim[0], dim[1], dedx_erange::size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        auto D = i.parent()->driver()->getSim()->get_dedx_calc().dedx();
                        mcinfo_data_node::dim_t d = { D.dim()[0], D.dim()[1], dedx_erange::size() };
                        for (uint i = 0; i < d[0]; ++i)
                            for (uint j = 0; j < d[1]; ++j) {
                                if (D(i, j)) {
                                    memcpy(s.data() + (i * d[1] + j) * d[2], D(i, j)->data().data(),
                                           dedx_erange::size() * sizeof(float));
                                }
                            }
                    });

            dedx.add_data(
                    "strag", "electronic straggling [eV], array [ions x materials x energy]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        auto &dim =
                                i.parent()->driver()->getSim()->get_dedx_calc().de_strag().dim();
                        d = { dim[0], dim[1], dedx_erange::size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        auto D = i.parent()->driver()->getSim()->get_dedx_calc().de_strag();
                        mcinfo_data_node::dim_t d = { D.dim()[0], D.dim()[1], dedx_erange::size() };
                        for (uint i = 0; i < d[0]; ++i)
                            for (uint j = 0; j < d[1]; ++j) {
                                if (D(i, j)) {
                                    memcpy(s.data() + (i * d[1] + j) * d[2], D(i, j)->data().data(),
                                           dedx_erange::size() * sizeof(float));
                                }
                            }
                    });

            mcinfo &flight_path = target.add_group("flight_path", "Flight path data");
            flight_path.add_data(
                    "erg", "flight path table energy grid [eV]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = { flight_path_calc::fp_tbl_erange::size() };
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        for (flight_path_calc::fp_tbl_iterator k; k < k.end(); k++)
                            s[k] = *k;
                    });
            flight_path.add_data(
                    "mfp", "mean free path [nm], array [ions x materials x energy]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = i.parent()->driver()->getSim()->get_fp_calc().mfp().dim();
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        ArrayNDf D = i.parent()->driver()->getSim()->get_fp_calc().mfp();
                        s.assign(D.data(), D.data() + D.size());
                    });
            flight_path.add_data(
                    "ipmax", "max impact parameter [nm], array [ions x materials x energy]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = i.parent()->driver()->getSim()->get_fp_calc().ipmax().dim();
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        ArrayNDf D = i.parent()->driver()->getSim()->get_fp_calc().ipmax();
                        s.assign(D.data(), D.data() + D.size());
                    });
            flight_path.add_data(
                    "fpmax", "max flight path [nm], array [ions x materials x energy]",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        d = i.parent()->driver()->getSim()->get_fp_calc().fpmax().dim();
                    },
                    [](const mcinfo_data_node &i, std::vector<float> &s) {
                        ArrayNDf D = i.parent()->driver()->getSim()->get_fp_calc().fpmax();
                        s.assign(D.data(), D.data() + D.size());
                    });
        }
    }

    // 3. ion_beam
    {
        mcinfo &ion_beam = add_group("ion_beam", "Ion beam parameters");
        ion_beam.add_data(
                "E0", "Mean energy [eV]",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<float> &s) {
                    s[0] = i.parent()
                                   ->driver()
                                   ->getSim()
                                   ->getSource()
                                   .getParameters()
                                   .energy_distribution.center;
                });
        ion_beam.add_data(
                "Z", "Atomic number",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<float> &s) {
                    s[0] = i.parent()
                                   ->driver()
                                   ->getSim()
                                   ->getSource()
                                   .getParameters()
                                   .ion.atomic_number;
                });
        ion_beam.add_data(
                "M", "Atomic mass [emu]",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                [](const mcinfo_data_node &i, std::vector<float> &s) {
                    s[0] = i.parent()
                                   ->driver()
                                   ->getSim()
                                   ->getSource()
                                   .getParameters()
                                   .ion.atomic_mass;
                });
        ion_beam.add_data(
                "dir0", "Mean direction",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 3 }; },
                [](const mcinfo_data_node &i, std::vector<float> &s) {
                    const auto &p = i.parent()->driver()->getSim()->getSource().getParameters();
                    s[0] = p.angular_distribution.center.x();
                    s[1] = p.angular_distribution.center.y();
                    s[2] = p.angular_distribution.center.z();
                });
        ion_beam.add_data(
                "x0", "Mean position",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 3 }; },
                [](const mcinfo_data_node &i, std::vector<float> &s) {
                    const auto &p = i.parent()->driver()->getSim()->getSource().getParameters();
                    s[0] = p.spatial_distribution.center.x();
                    s[1] = p.spatial_distribution.center.y();
                    s[2] = p.spatial_distribution.center.z();
                });
    }

    // 4. tally
    {
        mcinfo &tally_grp = add_group("tally", "Tally data");

        std::map<std::string, std::map<std::string, int>> tmap;
        int k = 1;
        while (k < tally::std_tallies) {
            tmap[tally::arrayGroup(k)][tally::arrayName(k)] = k;
            k++;
        }

        for (const auto &tgroup : tmap) {
            mcinfo &tg = tally_grp.add_group(tgroup.first, "");
            for (const auto &tname : tgroup.second) {
                int it = tname.second;
                tg.add_data(
                        tname.first, tally::arrayDescription(it),
                        [it](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                            d = i.parent()->driver()->getSim()->getTally().at(it).dim();
                        },
                        it);
            }
        }

        mcinfo &totals = tally_grp.add_group("totals", "Tally totals");
        totals.add_data(
                "data", "tally totals per atom",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                    d = i.parent()->driver()->getSim()->getTally().at(0).dim();
                },
                0);
        totals.add_data(
                "column_names", "names of totals",
                [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                    d = { i.parent()->driver()->getSim()->getTally().arrayNames().size() };
                },
                [](const mcinfo_data_node &i, std::vector<std::string> &s) {
                    s = i.parent()->driver()->getSim()->getTally().arrayNames();
                });
    }

    // 5. user_tally
    {
        mcinfo &ut_root = add_group("user_tally", "User-defined tallies");
        const auto &utv = driver_->getSim()->getUserTally();

        for (int iut = 0; iut < (int)utv.size(); ++iut) {
            const user_tally *ut = utv[iut];
            mcinfo &utg = ut_root.add_group(ut->id(), "");

            utg.add_data(
                    "data", "bin data per ion",
                    [iut](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        if (ut)
                            d = ut->data().dim();
                    },
                    -1, iut);

            utg.add_data(
                    "description", "Tally description",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [iut](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        s[0] = ut->description();
                    });

            {
                mcinfo &cs =
                        utg.add_group("coordinate_system", "Coordinate system of the user tally");
                cs.add_data(
                        "zaxis", "Vector parallel to the z-axis direction",
                        [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 3 }; },
                        [iut](const mcinfo_data_node &i, std::vector<float> &s) {
                            const user_tally *ut =
                                    i.parent()->driver()->getSim()->getUserTally()[iut];
                            const vector3 &v = ut->cs().zaxis;
                            s = { v.x(), v.y(), v.z() };
                        });
                cs.add_data(
                        "xzvector", "Vector on the xz-plane",
                        [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 3 }; },
                        [iut](const mcinfo_data_node &i, std::vector<float> &s) {
                            const user_tally *ut =
                                    i.parent()->driver()->getSim()->getUserTally()[iut];
                            const vector3 &v = ut->cs().xzvector;
                            s = { v.x(), v.y(), v.z() };
                        });
                cs.add_data(
                        "origin", "Coordinate system origin",
                        [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 3 }; },
                        [iut](const mcinfo_data_node &i, std::vector<float> &s) {
                            const user_tally *ut =
                                    i.parent()->driver()->getSim()->getUserTally()[iut];
                            const vector3 &v = ut->cs().origin;
                            s = { v.x(), v.y(), v.z() };
                        });
            }

            utg.add_data(
                    "event", "Tally event",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [iut](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        s[0] = event_name(ut->event());
                    });

            utg.add_data(
                    "event_description", "Tally event description",
                    [](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) { d = { 1 }; },
                    [iut](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        s[0] = event_description(ut->event());
                    });

            utg.add_data(
                    "bin_names", "Bin names",
                    [iut](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        d = { ut->bin_names().size() };
                    },
                    [iut](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        s = ut->bin_names();
                    });

            utg.add_data(
                    "bin_descriptions", "Bin descriptions",
                    [iut](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        d = { ut->bin_descriptions().size() };
                    },
                    [iut](const mcinfo_data_node &i, std::vector<std::string> &s) {
                        const user_tally *ut = i.parent()->driver()->getSim()->getUserTally()[iut];
                        s = ut->bin_descriptions();
                    });

            {
                mcinfo &bins = utg.add_group("bins", "Bin edges");
                std::vector<std::string> bin_names = ut->bin_names();
                for (int j = 0; j < (int)bin_names.size(); ++j) {
                    std::ostringstream os;
                    os << "Bin edges of dim " << j << " : [" << bin_names[j] << "]";
                    bins.add_data(
                            std::to_string(j), os.str(),
                            [iut, j](const mcinfo_data_node &i, mcinfo_data_node::dim_t &d) {
                                const user_tally *ut =
                                        i.parent()->driver()->getSim()->getUserTally()[iut];
                                d = { ut->bin_edges(j).size() };
                            },
                            [iut, j](const mcinfo_data_node &i, std::vector<float> &s) {
                                const user_tally *ut =
                                        i.parent()->driver()->getSim()->getUserTally()[iut];
                                s = ut->bin_edges(j);
                            });
                }
            }
        }
    }
}

bool mcinfo_data_node::get(std::vector<double> &s, std::vector<double> &ds) const
{
    const mcdriver *D = parent()->driver();
    if (D == nullptr || D->getSim() == nullptr)
        return false;

    const mccore *S = D->getSim();

    ArrayNDd A;
    ArrayNDd dA;
    if (extra_idx0_ >= 0) {
        A = S->getTally().at(extra_idx0_);
        dA = S->getTallyVar().at(extra_idx0_);
    } else {
        const user_tally *ut = S->getUserTally()[extra_idx1_];
        const user_tally *dut = S->getUserTallyVar()[extra_idx1_];
        if (!ut)
            return false;
        A = ut->data();
        dA = dut->data();
    }

    const size_t &N = S->ion_count();
    s.resize(A.size(), 0.0);
    ds.resize(A.size(), 0.0);
    if (N == 0)
        return true;

    for (int i = 0; i < A.size(); i++) {
        s[i] = A[i] / N;
        // error in the mean
        ds[i] = (N > 1) ? std::sqrt((dA[i] / N - s[i] * s[i]) / (N - 1)) : 0;
    }
    return true;
}
