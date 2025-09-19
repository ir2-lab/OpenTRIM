clear

# Load HDF5 Results
pkg load hdf5oct



D = h5load('usertally_test.h5','/user_tally/Test tally/data');

## Due to octave fortran style arrays, we have to transpose the N-D data array
n = ndims(D);
D = permute(D,n:-1:1);

labels = h5load('usertally_test.h5','/user_tally/Test tally/bin_names');
labels = h5load('usertally_test.h5','/user_tally/Test tally/bin_decriptions');

bins = {};

for i=1:n,
  bins{i} = h5load('usertally_test.h5',['/user_tally/Test tally/bins/' num2str(i-1)]);
end

figure 1
clf
d = squeeze(D(1,:,:));
x = bins{2};
y = bins{3};
x = 0.5*(x(1:end-1) + x(2:end));
y = 0.5*(y(1:end-1) + y(2:end));
[c,h]=contour(y,x,d);
title("500 keV Fe in Fe, Beam ions stopping in the material")
# clabel(c,h)
xlabel(labels{3})
ylabel(labels{2})
axis equal
colorbar('location','southoutside')
print -dpng usertally_test_fig1

figure 2
clf
d = squeeze(D(2,:,:));
x = bins{2};
y = bins{3};
x = 0.5*(x(1:end-1) + x(2:end));
y = 0.5*(y(1:end-1) + y(2:end));
[c,h]=contour(y,x,d);
title("500 keV Fe in Fe, Recoils stopping in the material")
# clabel(c,h)
xlabel(labels{3})
ylabel(labels{2})
axis equal
colorbar('location','southoutside')
print -dpng usertally_test_fig2



