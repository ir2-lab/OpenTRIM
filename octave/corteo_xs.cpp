#include <octave/oct.h>

#include "xs_corteo.h"

typedef xs_corteo4bit<Screening::ZBL> xs_t;
typedef xs_t::corteo_idx_t::e_index e_idx;
typedef xs_t::corteo_idx_t::s_index s_idx;

DEFUN_DLD(corteo_xs, args, nargout, "Returns corteo tabulated xs")
{

    xs_t xs;

    octave_idx_type ne = e_idx::size;
    octave_idx_type ns = s_idx::size;

    RowVector e(ne), s(ns);
    Matrix s2(ne, ns);

    {
        e_idx i;
        for (octave_idx_type k = 0; k < i.size; k++)
            e(k) = *i++;
    }
    {
        s_idx i;
        for (octave_idx_type k = 0; k < i.size; k++)
            s(k) = *i++;
    }
    e_idx ie;
    for (octave_idx_type ke = 0; ke < ne; ke++) {
        s_idx is;
        for (octave_idx_type ks = 0; ks < ns; ks++) {
            s2(ke, ks) = xs_t::sin2Thetaby2(ie, is++);
        }
        ie++;
    }

    octave_value_list retval(3);
    retval(0) = s2;
    retval(1) = e;
    retval(2) = s;
    return retval;
}
