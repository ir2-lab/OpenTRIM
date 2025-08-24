# -*- texinfo -*-
# @deftypefn {Function File} {@var{a}=} screening_length (@var{Z1}, @var{Z2}, @var{screening})
# @deftypefnx {Function File} {@var{a}=} screening_length (@var{Z1}, @var{Z2})
#
# Calculate the screenining length @var{a} in nm for the screened Coulomb
# interaction between ions of atomic number @var{Z1} and @var{Z2}.
#
# The optional 3rd argument @var{screening} is a string defining the type of screening
# potential and can take up one of the following values:
#   "None", "Bohr", "KrC", "Moliere", "ZBL"
#
# The default is "ZBL"
#
# @seealso{screened_coulomb_theta, screened_coulomb_xs, screened_coulomb_sn}
# @end deftypefn
#

function a = screening_length(Z1,Z2,screening)

if nargin<3,
  screening = "ZBL";
endif

if !ischar(screening),
  error("screening_length: 3rd arg must be a string");
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
  error("screening_length: 3rd arg is not a valid screening type");
endif

BOHR_RADIUS = 0.05291772108; # Bohr radius [nm]
SCREENCONST = 0.88534; # Lindhard screening length constant

switch (k),
  case {0}
    a = 1;
  case {1}
    a = SCREENCONST*BOHR_RADIUS./sqrt(Z1.^(2/3) + Z2.^(2/3));
  case {2,3,4,5}
    a = SCREENCONST*BOHR_RADIUS./(Z1.^(0.23) + Z2.^(0.23));
endswitch








