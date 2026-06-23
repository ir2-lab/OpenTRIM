# -*- texinfo -*-
# @deftypefn {Function File} {[@var{E}, @var{S}]=} dedx (@var{Z1},@var{Z2}, @var{X2})
#
# Calculate the electronic stopping power.
#
# This function calculates the stopping power table @var{S}=-dE/dx in eV/nm at
# as a function of projectile energy @var{E} for an ion with atomic
# number @var{Z1} traveling in a target containing atom(s) @var{Z2}
# with concentration(s) @var{X2} in at/nm^3.
#
# @end deftypefn
#

function [e, s] = dedx (Z1,Z2,X2,stopping_model)

if nargin<3,
  print_usage();
endif

if length(Z2)!=length(X2),
  error("dedx: Z2 and X2 must have equal length");
endif

if nargin<4,
  stopping_model = "SRIM13";
endif

if !ischar(stopping_model),
  error("dedx: 4th arg (stopping_model) must be a string");
endif

k=-1;
stopping_types = {"SRIM96", "SRIM13", "DPASS"};

for i=1:length(stopping_types)
  if strcmp(stopping_types{i},stopping_model),
    k = i-1;
    break;
  endif
endfor

if k<0,
  error("dedx: 4th arg is not a valid stopping model");
endif

N = sum(X2);
X2 = X2/N;

[e, s] = __dedx__(Z1,Z2,X2,N,k);

