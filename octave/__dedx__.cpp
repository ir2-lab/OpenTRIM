#include <octave/oct.h>

#include "dedx.h"

DEFUN_DLD(dedx, args, nargout, "Returns tables of electronic energy loss")
{
    if (args.length() != 2 || !args(0).is_real_scalar() || !args(1).is_real_scalar())
        print_usage();

    int Z1 = args(0).int_value(), Z2 = args(1).int_value();

    dedx_index i;
    octave_idx_type n = i.size;
    RowVector e(n), d(n);
    const float *p = raw_dedx(Z1, Z2);
    for (octave_idx_type k = 0; k < n; k++) {
        e(k) = *i++;
        d(k) = *p++;
    }

    octave_value_list retval(2);
    retval(0) = e;
    retval(1) = d;
    return retval;
}
