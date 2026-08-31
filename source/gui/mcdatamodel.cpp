#include "mcdatamodel.h"

#include "json_defs_p.h"

#include <mccore.h>

McDataModel::McDataModel(std::shared_ptr<mcdriver> d, QObject *parent)
    : QDataModel(d->config().Output.title.c_str(), parent), driver_(d)
{
    // 1. run_info
    addGroup("run_info");
    {
        addData(std::make_unique<TextDataSet>(
                        driver_, "title", "User supplied simulation title", 1,
                        [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                            s[0] = t->driver()->config().Output.title;
                        }),
                "/run_info");

        addData(std::make_unique<TextDataSet>(
                        driver_, "json_config", "JSON formatted simulation options", 1,
                        [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                            s[0] = t->driver()->config().toJSON();
                        }),
                "/run_info");

        addGroup("version_info", "/run_info");
        {
            addData(std::make_unique<TextDataSet>(
                            driver_, "name", "code name", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().project_name);
                            }),
                    "/run_info/version_info");
            addData(std::make_unique<TextDataSet>(
                            driver_, "version", "code version", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().version);
                            }),
                    "/run_info/version_info");
            addData(std::make_unique<TextDataSet>(
                            driver_, "git_tag", "git tag", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().git_tag);
                            }),
                    "/run_info/version_info");
            addData(std::make_unique<TextDataSet>(
                            driver_, "compiler", "code name", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().compiler_id);
                            }),
                    "/run_info/version_info");
            addData(std::make_unique<TextDataSet>(
                            driver_, "compiler_version", "compiler version", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().compiler_version);
                            }),
                    "/run_info/version_info");
            addData(std::make_unique<TextDataSet>(
                            driver_, "build_system", "build system", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().system_id);
                            }),
                    "/run_info/version_info");
            addData(std::make_unique<TextDataSet>(
                            driver_, "build_time", "build time", 1,
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                s[0] = std::string(mcdriver::version_info().build_time);
                            }),
                    "/run_info/version_info");
        }

        addData(std::make_unique<TextDataSet>(
                        driver_, "run_history", "run history", 1,
                        [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                            ojson j = t->driver()->run_history();
                            std::ostringstream os;
                            os << j.dump(4) << std::endl;
                            s[0] = os.str();
                        }),
                "/run_info");
    }

    // 2. target
    addGroup("target");
    {
        addGroup("grid", "/target");
        {
            const auto &g = d->getSim()->getTarget().grid();

            auto n = std::make_unique<NumericDataSet>("X", "x-axis grid [nm]",
                                                      AbstractDataSet::dim_t{ g.x().size() });
            int m = g.x().size();
            for (int i = 0; i < m; ++i)
                n->data[i] = g.x()[i];
            addData(std::move(n), "/target/grid");

            n = std::make_unique<NumericDataSet>("Y", "y-axis grid [nm]",
                                                 AbstractDataSet::dim_t{ g.y().size() });
            m = g.y().size();
            for (int i = 0; i < m; ++i)
                n->data[i] = g.y()[i];
            addData(std::move(n), "/target/grid");

            n = std::make_unique<NumericDataSet>("Z", "z-axis grid [nm]",
                                                 AbstractDataSet::dim_t{ g.z().size() });
            m = g.z().size();
            for (int i = 0; i < m; ++i)
                n->data[i] = g.z()[i];
            addData(std::move(n), "/target/grid");
        }

        const auto &mat = d->getSim()->getTarget().materials();
        AbstractDataSet::strvec_t matnames;
        for (const auto &m : mat)
            matnames.push_back(m->name());

        const auto &atoms = d->getSim()->getTarget().atoms();
        const auto &labels = d->getSim()->getTarget().atom_labels();

        addGroup("materials", "/target");
        if (!mat.empty()) {

            addData(std::make_unique<TextDataSet>(
                            driver_, "name", "name of material", mat.size(),
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                const auto &mat = t->driver()->getSim()->getTarget().materials();
                                for (int k = 0; k < s.size(); ++k)
                                    s[k] = mat[k]->name();
                            }),
                    "/target/materials");

            auto n = std::make_unique<NumericDataSet>(
                    "data", "material data", AbstractDataSet::dim_t{ mat.size(), 3 },
                    AbstractDataSet::strvec_t{ "material", "data" });
            int nmat = mat.size();
            n->x_category[0] = matnames;
            n->x_category[1] = { "atomic density [at/nm^3]", "mass density [g/cm^3]",
                                 "atomic radius [nm]" };
            for (int i = 0; i < nmat; ++i) {
                n->data(i, 0) = mat[i]->atomicDensity();
                n->data(i, 1) = mat[i]->massDensity();
                n->data(i, 2) = mat[i]->atomicRadius();
            }
            addData(std::move(n), "/target/materials");
        }

        addGroup("atoms", "/target");
        if (!atoms.empty()) {
            addData(std::make_unique<TextDataSet>(
                            driver_, "label", "label = [Atom (Chemical symbol)] in [Material]",
                            labels.size(),
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                const auto &labels =
                                        t->driver()->getSim()->getTarget().atom_labels();
                                for (int k = 0; k < s.size(); ++k)
                                    s[k] = labels[k];
                            }),
                    "/target/atoms");
            addData(std::make_unique<TextDataSet>(
                            driver_, "symbol", "Chemical symbol", labels.size(),
                            [](const TextDataSet *t, AbstractDataSet::strvec_t &s) {
                                const auto &atoms = t->driver()->getSim()->getTarget().atoms();
                                for (int k = 0; k < s.size(); ++k)
                                    s[k] = atoms[k]->symbol();
                            }),
                    "/target/atoms");

            auto n = std::make_unique<NumericDataSet>("data", "atom data",
                                                      AbstractDataSet::dim_t{ atoms.size(), 7 },
                                                      AbstractDataSet::strvec_t{ "atom", "data" });
            int natoms = atoms.size();
            n->x_category[0] = labels;
            n->x_category[1] = { "Atomic number",
                                 "Atomic mass [amu]",
                                 "Displacement energy [eV]",
                                 "Replacement energy [eV]",
                                 "Lattice Binding energy [eV]",
                                 "Surface binding energy [eV]",
                                 "Recombination radius [nm]" };
            for (int i = 0; i < natoms; ++i) {
                n->data(i, 0) = atoms[i]->Z();
                n->data(i, 1) = atoms[i]->M();
                n->data(i, 2) = atoms[i]->Ed();
                n->data(i, 3) = atoms[i]->Er();
                n->data(i, 4) = atoms[i]->El();
                n->data(i, 5) = atoms[i]->Es();
                n->data(i, 6) = atoms[i]->Rc();
            }
            addData(std::move(n), "/target/atoms");
        }

        addGroup("dedx", "/target");
        if (!mat.empty()) {
            size_t ne = dedx_erange::size();

            auto n = std::make_unique<NumericDataSet>(
                    "stopping", "Electronic stopping [eV/nm]",
                    AbstractDataSet::dim_t{ atoms.size(), mat.size(), ne },
                    AbstractDataSet::strvec_t{ "ion", "material", "E" },
                    AbstractDataSet::strvec_t{ "ion", "material", "Ion energy [eV]" });
            int natoms = atoms.size();
            int nmat = mat.size();
            n->x_category[0] = labels;
            n->x_category[1] = matnames;
            n->x[2].resize(ne);
            for (dedx_iterator k; k < k.end(); k++)
                n->x[2][k] = *k;
            const auto &D = d->getSim()->get_dedx_calc().dedx();
            for (int ia = 0; ia < natoms; ++ia)
                for (int im = 0; im < nmat; ++im)
                    if (D(ia, im)) {
                        double *v = &(n->data(ia, im, 0));
                        const float *p = D(ia, im)->data().data();
                        const double *vend = v + ne;
                        for (; v < vend; ++v, ++p)
                            *v = *p;
                    }
            addData(std::move(n), "/target/dedx");

            n = std::make_unique<NumericDataSet>(
                    "straggling", "Electronic straggling [eV/√nm]",
                    AbstractDataSet::dim_t{ atoms.size(), mat.size(), ne },
                    AbstractDataSet::strvec_t{ "ion", "material", "E" },
                    AbstractDataSet::strvec_t{ "ion", "material", "Ion energy [eV]" });
            n->x_category[0] = labels;
            n->x_category[1] = matnames;
            n->x[2].resize(ne);
            for (dedx_iterator k; k < k.end(); k++)
                n->x[2][k] = *k;
            const auto &Ds = d->getSim()->get_dedx_calc().de_strag();
            for (int ia = 0; ia < natoms; ++ia)
                for (int im = 0; im < nmat; ++im)
                    if (Ds(ia, im)) {
                        double *v = &(n->data(ia, im, 0));
                        const float *p = Ds(ia, im)->data().data();
                        const double *vend = v + ne;
                        for (; v < vend; ++v, ++p)
                            *v = *p;
                    }
            addData(std::move(n), "/target/dedx");
        }

        addGroup("flight_path", "/target");
        if (!mat.empty()) {
            size_t ne = flight_path_calc::fp_tbl_erange::size();

            auto n = std::make_unique<NumericDataSet>(
                    "mfp", "Mean free path [nm]",
                    AbstractDataSet::dim_t{ atoms.size(), mat.size(), ne },
                    AbstractDataSet::strvec_t{ "atom_id", "mat_id", "E" },
                    AbstractDataSet::strvec_t{ "Ion", "Material", "Ion energy [eV]" });
            n->x_category[0] = labels;
            n->x_category[1] = matnames;
            n->x[2].resize(ne);
            for (flight_path_calc::fp_tbl_iterator k; k < k.end(); k++)
                n->x[2][k] = *k;
            {
                const auto &D = d->getSim()->get_fp_calc().mfp();
                for (int i = 0; i < (int)D.size(); ++i)
                    n->data[i] = D[i];
            }
            addData(std::move(n), "/target/flight_path");

            n = std::make_unique<NumericDataSet>(
                    "ipmax", "Max impact parameter [nm]",
                    AbstractDataSet::dim_t{ atoms.size(), mat.size(), ne },
                    AbstractDataSet::strvec_t{ "atom_id", "mat_id", "E" },
                    AbstractDataSet::strvec_t{ "Ion", "Material", "Ion energy [eV]" });
            n->x_category[0] = labels;
            n->x_category[1] = matnames;
            n->x[2].resize(ne);
            for (flight_path_calc::fp_tbl_iterator k; k < k.end(); k++)
                n->x[2][k] = *k;
            {
                const auto &D = d->getSim()->get_fp_calc().ipmax();
                for (int i = 0; i < (int)D.size(); ++i)
                    n->data[i] = D[i];
            }
            addData(std::move(n), "/target/flight_path");

            n = std::make_unique<NumericDataSet>(
                    "fpmax", "Max flight path [nm]",
                    AbstractDataSet::dim_t{ atoms.size(), mat.size(), ne },
                    AbstractDataSet::strvec_t{ "atom_id", "mat_id", "E" },
                    AbstractDataSet::strvec_t{ "Ion", "Material", "Ion energy [eV]" });
            n->x_category[0] = labels;
            n->x_category[1] = matnames;
            n->x[2].resize(ne);
            for (flight_path_calc::fp_tbl_iterator k; k < k.end(); k++)
                n->x[2][k] = *k;
            {
                const auto &D = d->getSim()->get_fp_calc().fpmax();
                for (int i = 0; i < (int)D.size(); ++i)
                    n->data[i] = D[i];
            }
            addData(std::move(n), "/target/flight_path");
        }
    }

    // 3. ion beam
    addGroup("ion_beam");

    // 4. tally
    addGroup("tally");
    {
        std::map<std::string, std::map<std::string, int>> tmap;
        int k = 0;
        while (k < tally::std_tallies) {
            tmap[tally::arrayGroup(k)][tally::arrayName(k)] = k;
            k++;
        }

        for (const auto &tgroup : tmap) {
            QString gloc = QString("/tally/") + tgroup.first.c_str();
            addGroup(tgroup.first.c_str(), "/tally");
            for (const auto &tname : tgroup.second) {
                int it = tname.second;
                addData(std::make_unique<TallyDataSet>(driver_, d->getSim()->getTally(),
                                                       d->getSim()->getTallyVar(),
                                                       static_cast<tally::tally_t>(it)),
                        gloc);
            }
        }
    }

    // 5. user_tally
    if (!d->getSim()->getUserTally().empty()) {
        addGroup("user_tally");
        {
            const auto &utv = d->getSim()->getUserTally();
            const auto &dutv = d->getSim()->getUserTallyVar();

            for (int iut = 0; iut < utv.size(); ++iut) { // only if a user tally has been defined

                const user_tally *ut = utv[iut];
                const user_tally *dut = dutv[iut];

                addData(std::make_unique<UTallyDataSet>(driver_, ut, dut), "/user_tally");
            }
        }
    }
}

TallyDataSet::TallyDataSet(std::shared_ptr<mcdriver> d, const tally &t, const tally &dt,
                           tally::tally_t type)
    : driver_(d), data_(t.at(type)), errors_(dt.at(type)), type_(type)
{
    dim_ = data_.dim();
    name_ = t.arrayName(type);
    desc_ = t.arrayDescription(type);
    arrayNames_ = t.arrayNames();
    atomLabels_ = d->getSim()->getTarget().atom_labels();

    if (type == tally::cT) {
        dim_name_.push_back("tally_id");
        dim_name_.push_back("atom_id");
        dim_desc_.push_back("id of tally table");
        dim_desc_.push_back("id of atom");
    } else {
        dim_name_.push_back("atom_id");
        dim_name_.push_back("x");
        dim_name_.push_back("y");
        dim_name_.push_back("z");
        dim_desc_.push_back("id of atom");
        dim_desc_.push_back("x [nm]");
        dim_desc_.push_back("y [nm]");
        dim_desc_.push_back("z [nm]");
    }
}

size_t TallyDataSet::get_x_categorical(size_t d, strvec_t &x) const
{
    if (type_ == tally::cT) {
        if (d == 0) {
            x = arrayNames_;
            return x.size();
        } else {
            x = atomLabels_;
            return x.size();
        }
    } else {
        if (d == 0) {
            x = atomLabels_;
            return x.size();
        } else {
            return 0;
        }
    }
}

size_t TallyDataSet::get_x(size_t d, size_t n, double *v) const
{
    switch (d) {
    case 0:
        return AbstractDataSet::get_x(d, n, v);
    case 1: {
        const auto &x = driver_->getSim()->getTarget().grid().x();
        size_t m = std::min(n, x.size());
        double *vend = v + m;
        auto it = x.begin();
        for (; v < vend; ++v, ++it)
            *v = *it;
        return m;
    }
    case 2: {
        const auto &x = driver_->getSim()->getTarget().grid().y();
        size_t m = std::min(n, x.size());
        double *vend = v + m;
        auto it = x.begin();
        for (; v < vend; ++v, ++it)
            *v = *it;
        return m;
    }
    case 3: {
        const auto &x = driver_->getSim()->getTarget().grid().z();
        size_t m = std::min(n, x.size());
        double *vend = v + m;
        auto it = x.begin();
        for (; v < vend; ++v, ++it)
            *v = *it;
        return m;
    }
    default:
        return 0;
    }

    return 0;
}

size_t TallyDataSet::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
{
    if (data_.isNull())
        return 0;
    size_t N = driver_->getSim()->ion_count();
    size_t m = data_.dim()[d]; // size of dimension d
    size_t s = data_.stride()[d]; // stride of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = &data_(i0); // pointer to 1st data value
    const double *vend = v + m;
    for (; v < vend; ++v, p += s)
        *v = *p / N;
    return m;
}

size_t TallyDataSet::get_dy(size_t d, const dim_t &i0, size_t n, double *v) const
{
    if (errors_.isNull() || data_.isNull())
        return 0;
    size_t N = driver_->getSim()->ion_count();
    size_t m = data_.dim()[d]; // size of dimension d
    size_t s = data_.stride()[d]; // stride of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = &data_(i0); // pointer to 1st data value
    const double *dp = &errors_(i0); // pointer to 1st error value
    const double *vend = v + m;
    for (; v < vend; ++v, p += s, dp += s) {
        double x = *p / N;
        double dx = (N > 1) ? std::sqrt((*dp / N - x * x) / (N - 1)) : 0;
        *v = dx;
    }
    return m;
}

UTallyDataSet::UTallyDataSet(std::shared_ptr<mcdriver> d, const user_tally *t, const user_tally *dt)
    : driver_(d), t_(t), dt_(dt), data_(t->data()), errors_(dt->data())
{
    dim_ = data_.dim();
    name_ = t_->id();
    {
        desc_ = "Event: ";
        desc_ += event_name(t_->event());
        desc_ += ": ";
        desc_ += event_description(t_->event());
    }
    dim_name_ = t_->bin_names();
    dim_desc_ = t_->bin_descriptions();

    atomLabels_ = d->getSim()->getTarget().atom_labels();
}

size_t UTallyDataSet::get_x(size_t d, size_t n, double *v) const
{
    const auto &x = t_->bin_edges(d);
    size_t m = std::min(n, x.size());
    double *vend = v + m;
    auto it = x.begin();
    for (; v < vend; ++v, ++it)
        *v = *it;
    return m;
}

size_t UTallyDataSet::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
{
    if (data_.isNull())
        return 0;
    size_t N = driver_->getSim()->ion_count();
    size_t m = data_.dim()[d]; // size of dimension d
    size_t s = data_.stride()[d]; // stride of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = &data_(i0); // pointer to 1st data value
    const double *vend = v + m;
    for (; v < vend; ++v, p += s)
        *v = *p / N;
    return m;
}

size_t UTallyDataSet::get_dy(size_t d, const dim_t &i0, size_t n, double *v) const
{
    if (errors_.isNull() || data_.isNull())
        return 0;
    size_t N = driver_->getSim()->ion_count();
    size_t m = data_.dim()[d]; // size of dimension d
    size_t s = data_.stride()[d]; // stride of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = &data_(i0); // pointer to 1st data value
    const double *dp = &errors_(i0); // pointer to 1st error value
    const double *vend = v + m;
    for (; v < vend; ++v, p += s, dp += s) {
        double x = *p / N;
        *v = (N > 1) ? std::sqrt((*dp / N - x * x) / (N - 1)) : 0;
    }
    return m;
}

TextDataSet::TextDataSet(std::shared_ptr<mcdriver> d, const std::string &name,
                         const std::string &desc, size_t dim, const getter_t &g,
                         const strvec_t &x_category)
    : AbstractDataSet(name, { dim }), driver_(d), x_category_(x_category), getter_(g)
{
    desc_ = desc;
}

size_t NumericDataSet::get_x(size_t d, size_t n, double *v) const
{
    if (x[d].empty())
        return AbstractDataSet::get_x(d, n, v);
    size_t m = dim_[d]; // size of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = x[d].data(); // pointer to 1st data value
    const double *vend = v + m;
    for (; v < vend; ++v, ++p)
        *v = *p;
    return m;
}

size_t NumericDataSet::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
{
    if (data.isNull())
        return 0;

    size_t m = dim_[d]; // size of dimension d
    m = std::min(n, m); // only copy n values

    size_t s = data.stride()[d];
    const double *p = &(data(i0));
    const double *vend = v + m;
    for (; v < vend; ++v, p += s)
        *v = *p;
    return m;
}
