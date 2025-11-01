#include "mcplotinfo.h"

#include <mccore.h>

mcplotinfo::mcplotinfo(const McDriverObj *d)
{
    // 1. run_info
    mcplotinfo *p = add_child("run_info", new mcplotinfo);
    {
        TextDataStore *t =
                new TextDataStore(d, "title", "User supplied simulation title", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = t->driver()->options().Output.title;
                                  });
        p->add_child("title", new mcplotinfo(t));

        t = new TextDataStore(d, "json_config", "JSON formatted simulation options", 1,
                              [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                  s[0] = t->driver()->options().toJSON();
                              });
        p->add_child("json_config", new mcplotinfo(t));

        mcplotinfo *p1 = p->add_child("version_info", new mcplotinfo);
        {
            t = new TextDataStore(d, "name", "code name", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = std::string(PROJECT_NAME);
                                  });
            p1->add_child("version", new mcplotinfo(t));
            t = new TextDataStore(d, "version", "code version", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = std::string(PROJECT_VERSION);
                                  });
            p1->add_child("version", new mcplotinfo(t));
            t = new TextDataStore(d, "compiler", "code name", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = std::string(COMPILER_ID);
                                  });
            p1->add_child("compiler", new mcplotinfo(t));
            t = new TextDataStore(d, "compiler_version", "compiler version", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = std::string(COMPILER_VERSION);
                                  });
            p1->add_child("compiler_version", new mcplotinfo(t));
            t = new TextDataStore(d, "build_system", "build system", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = std::string(SYSTEM_ID);
                                  });
            p1->add_child("build_system", new mcplotinfo(t));
            t = new TextDataStore(d, "build_time", "build time", 1,
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      s[0] = std::string(BUILD_TIME);
                                  });
            p1->add_child("build_time", new mcplotinfo(t));
        }

        t = new TextDataStore(d, "run_history", "run history", 1,
                              [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                  ojson j = t->driver()->get_mcdriver()->run_history();
                                  std::ostringstream os;
                                  os << j.dump(4) << std::endl;
                                  s[0] = os.str();
                              });
        p->add_child("run_history", new mcplotinfo(t));
    }

    // 2. target
    p = add_child("target", new mcplotinfo);
    {
        mcplotinfo *p1;
        p1 = p->add_child("grid", new mcplotinfo);
        {
            const auto &g = d->getSim()->getTarget().grid();

            NumericDataStore *n = new NumericDataStore("X", "x-axis grid [nm]", { g.x().size() });
            int m = g.x().size();
            for (int i = 0; i < m; ++i)
                n->data[i] = g.x()[i];
            p1->add_child("X", new mcplotinfo(n));

            n = new NumericDataStore("Y", "y-axis grid [nm]", { g.y().size() });
            m = g.y().size();
            for (int i = 0; i < m; ++i)
                n->data[i] = g.y()[i];
            p1->add_child("Y", new mcplotinfo(n));

            n = new NumericDataStore("Z", "z-axis grid [nm]", { g.z().size() });
            m = g.z().size();
            for (int i = 0; i < m; ++i)
                n->data[i] = g.z()[i];
            p1->add_child("Z", new mcplotinfo(n));
        }

        const auto &mat = d->getSim()->getTarget().materials();
        AbstractDataStore::strvec_t matnames;
        for (const auto &m : mat)
            matnames.push_back(m->name());

        const auto &atoms = d->getSim()->getTarget().atoms();
        const auto &labels = d->getSim()->getTarget().atom_labels();

        p1 = p->add_child("materials", new mcplotinfo);
        {

            TextDataStore *t =
                    new TextDataStore(d, "name", "name of material", mat.size(),
                                      [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                          const auto &mat =
                                                  t->driver()->getSim()->getTarget().materials();
                                          for (int k = 0; k < s.size(); ++k)
                                              s[k] = mat[k]->name();
                                      });
            p1->add_child("name", new mcplotinfo(t));

            NumericDataStore *n = new NumericDataStore("data", "material data", { mat.size(), 3 },
                                                       { "material", "data" });
            int nmat = mat.size();
            n->x_category[0] = matnames;
            n->x_category[1] = { "atomic density [at/nm^3]", "mass density [g/cm^3]",
                                 "atomic radius [nm]" };
            for (int i = 0; i < nmat; ++i) {
                n->data(i, 0) = mat[i]->atomicDensity();
                n->data(i, 1) = mat[i]->massDensity();
                n->data(i, 2) = mat[i]->atomicRadius();
            }
            p1->add_child("data", new mcplotinfo(n));
        }
        p1 = p->add_child("atoms", new mcplotinfo);
        {
            TextDataStore *t = new TextDataStore(
                    d, "label", "label = [Atom (Chemical symbol)] in [Material]", labels.size(),
                    [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                        const auto &labels = t->driver()->getSim()->getTarget().atom_labels();
                        for (int k = 0; k < s.size(); ++k)
                            s[k] = labels[k];
                    });
            p1->add_child("label", new mcplotinfo(t));
            t = new TextDataStore(d, "symbol", "Chemical symbol", labels.size(),
                                  [](const TextDataStore *t, AbstractDataStore::strvec_t &s) {
                                      const auto &atoms =
                                              t->driver()->getSim()->getTarget().atoms();
                                      for (int k = 0; k < s.size(); ++k)
                                          s[k] = atoms[k]->symbol();
                                  });
            p1->add_child("symbol", new mcplotinfo(t));

            NumericDataStore *n = new NumericDataStore("data", "atom data", { atoms.size(), 7 },
                                                       { "atom", "data" });
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
            p1->add_child("data", new mcplotinfo(n));
        }

        p1 = p->add_child("Electronic effects", new mcplotinfo);
        {
            size_t ne = dedx_erange::size();

            NumericDataStore *n = new NumericDataStore(
                    "dEdx", "electronic stopping [eV/nm]", { atoms.size(), mat.size(), ne },
                    { "ion", "material", "E" }, { "ion", "material", "Ion energy [eV]" });
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
            p1->add_child("dEdx", new mcplotinfo(n));

            n = new NumericDataStore("Ω", "Straggling [eV/√nm]", { atoms.size(), mat.size(), ne },
                                     { "ion", "material", "E" },
                                     { "ion", "material", "Ion energy [eV]" });
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
            p1->add_child("Ω", new mcplotinfo(n));
        }

        p1 = p->add_child("flight_path", new mcplotinfo);
        {
            size_t ne = flight_path_calc::fp_tbl_erange::size();

            NumericDataStore *n = new NumericDataStore(
                    "mfp", "Mean free path [nm]", { atoms.size(), mat.size(), ne },
                    { "atom_id", "mat_id", "E" }, { "Ion", "Material", "Ion energy [eV]" });
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
            p1->add_child("mfp", new mcplotinfo(n));

            n = new NumericDataStore("ipmax", "Max impact parameter [nm]",
                                     { atoms.size(), mat.size(), ne }, { "atom_id", "mat_id", "E" },
                                     { "Ion", "Material", "Ion energy [eV]" });
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
            p1->add_child("ipmax", new mcplotinfo(n));

            n = new NumericDataStore("fpmax", "Max flight path [nm]",
                                     { atoms.size(), mat.size(), ne }, { "atom_id", "mat_id", "E" },
                                     { "Ion", "Material", "Ion energy [eV]" });
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
            p1->add_child("fpmax", new mcplotinfo(n));
        }
    }

    // 3. ion beam
    p = add_child("ion_beam", new mcplotinfo);
    {
    }

    // 4. tally
    p = add_child("tally", new mcplotinfo);
    {
        std::map<std::string, std::map<std::string, int>> tmap;
        int k = 0;
        while (k < tally::std_tallies) {
            tmap[tally::arrayGroup(k)][tally::arrayName(k)] = k;
            k++;
        }

        for (const auto &tgroup : tmap) {
            mcplotinfo *p1 = p->add_child(tgroup.first, new mcplotinfo);
            for (const auto &tname : tgroup.second) {
                int it = tname.second;
                p1->add_child(tname.first,
                              new mcplotinfo(new TallyDataStore(d, d->getSim()->getTally(),
                                                                d->getSim()->getTallyVar(),
                                                                static_cast<tally::tally_t>(it))));
            }
        }
    }

    // 5. user_tally
    if (!d->getSim()->getUserTally().empty()) {
        p = add_child("user_tally", new mcplotinfo);
        {
            const auto &utv = d->getSim()->getUserTally();
            const auto &dutv = d->getSim()->getUserTallyVar();

            for (int iut = 0; iut < utv.size(); ++iut) { // only if a user tally has been defined

                const user_tally *ut = utv[iut];
                const user_tally *dut = dutv[iut];

                mcplotinfo *p1 =
                        p->add_child(ut->id(), new mcplotinfo(new UTallyDataStore(d, ut, dut)));
            }
        }
    }
}

TallyDataStore::TallyDataStore(const McDriverObj *d, const tally &t, const tally &dt,
                               tally::tally_t type)
    : driver_(d), data_(t.at(type)), errors_(dt.at(type)), type_(type)
{
    dim_ = data_.dim();
    if (type != tally::cT) {
        dim_[1]--;
        dim_[2]--;
        dim_[3]--;
    }
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

size_t TallyDataStore::get_x_categorical(size_t d, strvec_t &x) const
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

size_t TallyDataStore::get_x(size_t d, size_t n, double *v) const
{
    switch (d) {
    case 0:
        return AbstractDataStore::get_x(d, n, v);
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

size_t TallyDataStore::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
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

size_t TallyDataStore::get_dy(size_t d, const dim_t &i0, size_t n, double *v) const
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

UTallyDataStore::UTallyDataStore(const McDriverObj *d, const user_tally *t, const user_tally *dt)
    : driver_(d), t_(t), dt_(dt), data_(t->data()), errors_(dt->data())
{
    dim_ = data_.dim();
    for (int i = 0; i < (int)dim_.size(); ++i)
        dim_[i]--;
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

size_t UTallyDataStore::get_x(size_t d, size_t n, double *v) const
{
    const auto &x = t_->bin_edges(d);
    size_t m = std::min(n, x.size());
    double *vend = v + m;
    auto it = x.begin();
    for (; v < vend; ++v, ++it)
        *v = *it;
    return m;
}

size_t UTallyDataStore::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
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

size_t UTallyDataStore::get_dy(size_t d, const dim_t &i0, size_t n, double *v) const
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

TextDataStore::TextDataStore(const McDriverObj *d, const std::string &name, const std::string &desc,
                             size_t dim, const getter_t &g, const strvec_t &x_category)
    : AbstractDataStore(name, { dim }), driver_(d), x_category_(x_category), getter_(g)
{
    desc_ = desc;
}

size_t NumericDataStore::get_x(size_t d, size_t n, double *v) const
{
    if (x[d].empty())
        return AbstractDataStore::get_x(d, n, v);
    size_t m = dim_[d]; // size of dimension d
    m = std::min(n, m); // only copy n values
    const double *p = x[d].data(); // pointer to 1st data value
    const double *vend = v + m;
    for (; v < vend; ++v, ++p)
        *v = *p;
    return m;
}

size_t NumericDataStore::get_y(size_t d, const dim_t &i0, size_t n, double *v) const
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
