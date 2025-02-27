clear

prefix  = [get_home_directory() '/.local/'];
eigendir = '/usr/include/eigen3';

# libdir = [prefix 'lib'];

mkoctfile(['-I' prefix 'include/opentrim'],...
          ['-I' eigendir],...
          ['-L' prefix 'lib'],...
          '-ldedx',...
          ['-Wl,-rpath=' prefix 'lib'],...
          '-v','dedx.cpp')

mkoctfile(['-I' prefix 'include/opentrim'],...
          ['-I' eigendir],...
          '-v','screened_coulomb_theta.cpp')

mkoctfile(['-I' prefix 'include/opentrim'],...
          ['-I' eigendir],...
          '-v','screened_coulomb_ip.cpp')

mkoctfile(['-I' prefix 'include/opentrim'],...
          ['-I' eigendir],...
          '-v','screened_coulomb_xs.cpp')

mkoctfile(['-I' prefix 'include/opentrim'],...
          ['-I' eigendir],...
          '-v','screened_coulomb_sn.cpp')

mkoctfile(['-I' prefix 'include/opentrim'],...
          ['-I' eigendir],...
          ['-L' prefix 'lib'],...
          '-lxs_zbl',...
          ['-Wl,-rpath=' prefix 'lib'],...
          '-v','corteo_xs.cpp')


