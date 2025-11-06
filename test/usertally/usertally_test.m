clear

# Load HDF5 Results
pkg load hdf5oct

file_name = 'usertally_test.h5';
user_tally_id = {'Implanted Fe', 'Fe Interstitials'};

for iut = 1:2,

  path = ['/user_tally/' user_tally_id{iut}];
  D = h5load(file_name,[path '/data']);
  id = h5load(file_name,[path '/data']);

  ## Due to octave fortran style arrays, we have to transpose the N-D data array
  n = ndims(D);
  D = permute(D,n:-1:1);

  labels = h5load(file_name,[path '/bin_names']);
  labels = h5load(file_name,[path '/bin_descriptions']);

  bins = {};

  for i=1:length(labels),
    bins{i} = h5load(file_name,[path '/bins/' num2str(i-1)]);
  end

  figure(iut)
  clf
  x = bins{1};   
  y = bins{2}; 
  contour(x,y,D(:,:,1).');
  title(["500 keV Fe in Fe, " user_tally_id{iut}])
  xlabel(labels{1})
  ylabel(labels{2})
  axis equal
  colorbar('location','southoutside')

  print('-dpng',['usertally_test_fig' num2str(iut)]);

end




