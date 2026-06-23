# -*- texinfo -*-
# @deftypefn {Function File} {@var{s}=} screened_coulomb_ip (@var{e}, @var{theta}, @var{screening})
# @deftypefnx {Function File} {@var{s}=} screened_coulomb_ip (@var{e}, @var{theta})
#
# Calculate the screened Coulomb potential reduced cross-section.
#
# @var{e} and @var{theta} are the reduced energy and center-of-mass
# scattering angle, respectively.
# They can be either both arrays with equal number of elements N or one of them a scalar
# and the other an array. A corresponding array of reduced impact parameter values is returned
# in @var{s}.
#
# The optional 3rd argument @var{screening} is a string defining the type of screening
# potential and can take up one of the following values:
#   "None", "Bohr", "KrC", "Moliere", "ZBL"
#
# The default is "ZBL"
#
# @seealso{screened_coulomb_theta, screened_coulomb_ip, screened_coulomb_sn}
# @end deftypefn
#

function th = screened_coulomb_xs(e,s,screening)

ne = length(e);
ns = length(s);

if (ne>1) && (ns>1) && (ne!=ns)
  error("screened_coulomb_xs: 1st and 2nd args different number of elements");
endif

if nargin<3,
  screening = "ZBL";
endif

if !ischar(screening),
  error("screened_coulomb_xs: 3rd arg must be a string");
endif

k=-1;
screening_types = {"None", "Bohr", "KrC", "Moliere", "ZBL"};

for i=1:length(screening_types)
  if strcmp(screening_types{i},screening),
    k = i-1;
    break;
  endif
endfor

if k<0,
  error("screened_coulomb_xs: 3rd arg is not a valid screening type");
endif

th = __screened_coulomb_xs__(e,s,k);




