#include <octave/oct.h>
#include <sstream>

#include <mcdriver.h>
#include <mcinfo.h>

#include "class_handle.h"

// Build a dim_vector from mcinfo::dim_t.
// 0-d or 1-d data becomes a (1×N) row vector; 2-d+ stays as-is.
static dim_vector make_dim_vector(const mcinfo_data_node::dim_t &dim, size_t data_size)
{
    if (dim.empty())
        return dim_vector(1, static_cast<octave_idx_type>(data_size));
    if (dim.size() == 1)
        return dim_vector(1, static_cast<octave_idx_type>(dim[0]));

    dim_vector dv;
    int n = static_cast<int>(dim.size());
    dv.resize(n);
    // copy dims in reverse order (OpenTRIM: C storage, Octave: Fortran storage !!exit)
    for (int i = 0; i < n; ++i)
        dv(n - i - 1) = static_cast<octave_idx_type>(dim[i]);
    return dv;
}

DEFUN_DLD(__opentrim_info__, args, /* nargout */, "opentrim.info dispatcher — do not call directly")
{
    octave_value_list retval;

    if (args.length() < 1 || !args(0).is_string())
        error("__opentrim_info__: first argument must be a command string");

    const std::string cmd = args(0).string_value();

    try {
        if (cmd == "new") {
            // args(1) = driver handle (uint64 → shared mcdriver*)
            auto d = shared_handle_obj<mcdriver>(args(1));
            retval(0) = handle_create(new mcinfo(d));

        } else if (cmd == "delete") {
            handle_destroy<mcinfo>(args(1));

        } else if (cmd == "is_group") {
            // is_group(handle)
            // is_group(handle,path)
            // args(1) = driver handle (uint64 → shared mcdriver*)
            // args(2) = path to node [optional]
            const mcinfo *root = handle_obj<mcinfo>(args(1));
            if (args.length() > 2) {
                const std::string path = args(2).string_value();
                const mcinfo_node *node = root->path2node(path);
                if (!node)
                    error("__opentrim_info__: invalid path '%s'", path.c_str());
                retval(0) = node->is_group();
            } else {
                retval(0) = root->is_group();
            }

        } else if (cmd == "get") {
            // args: (handle, path, nargout_int)
            if (args.length() < 3)
                error("opentrim.info.get: requires (handle, path)");
            const mcinfo *root = handle_obj<mcinfo>(args(1));
            const std::string path = args(2).string_value();
            int nout = (args.length() > 3) ? args(3).int_value() : 1;

            const mcinfo_node *node = root->path2node(path);
            if (!node)
                error("__opentrim_info__: invalid path '%s'", path.c_str());

            if (node->is_group()) {
                // Return a handle to the sub-tree.
                retval(0) = handle_create((mcinfo *)node, false);
                return retval;
            }

            const mcinfo_data_node *dnode = (mcinfo_data_node *)node;

            switch (dnode->type()) {

            case mcinfo_data_node::string:
            case mcinfo_data_node::json: {
                std::vector<std::string> sv;
                dnode->get(sv);
                if (sv.size() == 1) {
                    retval(0) = octave_value(sv[0]);
                } else {
                    Cell cell(1, static_cast<int>(sv.size()));
                    for (size_t i = 0; i < sv.size(); ++i)
                        cell(i) = octave_value(sv[i]);
                    retval(0) = octave_value(cell);
                }
                break;
            }

            case mcinfo_data_node::real64: {
                std::vector<double> v;
                dnode->get(v);
                dim_vector dv = make_dim_vector(dnode->dim(), v.size());
                NDArray arr(dv);
                for (size_t i = 0; i < v.size(); ++i)
                    arr(i) = v[i];
                retval(0) = octave_value(arr);
                break;
            }

            case mcinfo_data_node::real32: {
                std::vector<float> v;
                dnode->get(v);
                dim_vector dv = make_dim_vector(dnode->dim(), v.size());
                FloatNDArray arr(dv);
                for (size_t i = 0; i < v.size(); ++i)
                    arr(i) = v[i];
                retval(0) = octave_value(arr);
                break;
            }

            case mcinfo_data_node::uint64: {
                std::vector<uint64_t> v;
                dnode->get(v);
                dim_vector dv = make_dim_vector(dnode->dim(), v.size());
                uint64NDArray arr(dv);
                for (size_t i = 0; i < v.size(); ++i)
                    arr(i) = octave_uint64(v[i]);
                retval(0) = octave_value(arr);
                break;
            }

            case mcinfo_data_node::tally_score: {
                std::vector<double> x, dx;
                dnode->get(x, dx);
                dim_vector dv = make_dim_vector(dnode->dim(), x.size());
                NDArray val_arr(dv), err_arr(dv);
                for (size_t i = 0; i < x.size(); ++i) {
                    val_arr(i) = x[i];
                    err_arr(i) = dx[i];
                }
                retval(0) = octave_value(val_arr);
                if (nout >= 2)
                    retval(1) = octave_value(err_arr);
                break;
            }

            default:
                error("opentrim.info.get: invalid node at path '%s'", path.c_str());
            }

        } else if (cmd == "description") {
            const mcinfo *root = handle_obj<mcinfo>(args(1));
            if (args.length() > 2) {
                const std::string path = args(2).string_value();
                const mcinfo_node *node = root->path2node(path);
                retval(0) = octave_value(node->description());
            } else
                retval(0) = octave_value(root->description());

        } else if (cmd == "dump") {
            const mcinfo *root = handle_obj<mcinfo>(args(1));
            const std::string path = args(2).string_value();
            const mcinfo_node *node = root->path2node(path);
            std::ostringstream ret;
            ret << node->name() << ", ";
            if (node->is_group()) {
                ret << "[Group], " << node->description();
            } else {
                const mcinfo_data_node *dnode = (mcinfo_data_node *)node;
                auto dim = dnode->dim();

                switch (dnode->type()) {

                case mcinfo_data_node::string:
                case mcinfo_data_node::json: {
                    ret << "string, ";
                    break;
                }

                case mcinfo_data_node::real64: {
                    ret << "real64, ";
                    break;
                }

                case mcinfo_data_node::real32: {
                    ret << "real32, ";
                    break;
                }

                case mcinfo_data_node::uint64: {
                    ret << "uint64, ";
                    break;
                }

                case mcinfo_data_node::tally_score: {
                    ret << "[tally], ";
                    break;
                }

                default:
                    error("opentrim.info.get: invalid node at path '%s'", path.c_str());
                }

                if (dnode->size() == 0)
                    ret << "empty, ";
                if (dnode->size() == 1)
                    ret << "Scalar, ";
                else {
                    ret << "[" << dim[0];
                    for (int i = 1; i < dim.size(); ++i) {
                        ret << "×" << dim[i];
                    }
                    ret << "], ";
                }
                ret << dnode->description();
            }
            retval(0) = octave_value(ret.str());

        } else if (cmd == "keys") {
            const mcinfo *root = handle_obj<mcinfo>(args(1));
            auto k = root->keys();
            Cell cell(1, static_cast<int>(k.size()));
            for (size_t i = 0; i < k.size(); ++i)
                cell(i) = octave_value(k[i]);
            retval(0) = octave_value(cell);

        } else {
            error("__opentrim_info__: unknown command '%s'", cmd.c_str());
        }

    } catch (const std::exception &e) {
        error("opentrim.info: %s", e.what());
    }

    return retval;
}
