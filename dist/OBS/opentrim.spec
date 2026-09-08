# set next variable to 1 if debug package is needed, else leave it to nil
# only for rpm builds
%define         debug_build                 %{nil}
%global         __strip                     /bin/true
%global         _lto_cflags                 %{nil}

###############################################################################################################################
# specific configurations/settings per distribution
#
# python3 executable and sitearch macros
%define         __python_package           python3
%define         __my_python_exe            %{__python3}
%define         __my_python_site           %{python3_sitearch}

%if 0%{?is_opensuse} && 0%{?sle_version} == 150600
%define         __gcc_min_version          11
%endif

###############################################################################################################################

Name:           opentrim
Version:	       0
Release:	       0
Summary:	       Ion transport simulation in materials
License:	       MIT
Url:		       https://github.com/ir2-lab/OpenTRIM.git

%if "%{_vendor}" == "debbuild"
Packager:       M. Axiotis <psaxioti@gmail.com>
   %if 0%{?ubuntu_version} >= 2204 || 0%{?debian_version} >= 1100
BuildRequires:  debhelper-compat = 13
   %else
BuildRequires:  debhelper-compat = 12
   %endif
BuildRequires:  debbuild-macros

BuildRequires:  qtbase5-dev
BuildRequires:	 hdf5-tools
BuildRequires:	 libqt5svg5-dev
%else
BuildRequires:  python-rpm-macros
%endif

Source0:	       %{name}.tar.gz

Source11:       ext1.tar.gz
Source12:       ext2.tar.gz
Source13:       ext3.tar.gz
Source14:       ext4.tar.gz
Source15:       ext5.tar.gz
Source17:       ext7.tar.gz
Source18:       ext8.tar.gz
Source19:       ext9.tar.gz

BuildRequires:	 cmake >= 3.23
BuildRequires:	 libdedx-devel

BuildRequires:  %{!?_debbuild:gcc%{?__gcc_min_version:%{__gcc_min_version}}-c++}     %{?_debbuild:g++}
BuildRequires:  %{!?_debbuild:hdf5-devel}      %{?_debbuild:libhdf5-dev}
BuildRequires:  %{!?_debbuild:eigen3-devel}      %{?_debbuild:libeigen3-dev}  >= 3.4

%if ( 0%{?sle_version} == 150500 || 0%{?sle_version} == 150600 ) && 0%{?is_opensuse}
BuildRequires:  libQtDataBrowser0-devel
BuildRequires:  libQMatPlotWidget0-devel
BuildRequires:  libQtVectorEdit0-devel
%else
BuildRequires:  qtdatabrowser-devel
BuildRequires:  qmatplotwidget-devel
BuildRequires:  qtvectoredit-devel
%endif
%if 0%{?centos_version} && 0%{?centos_version} == 1000
BuildRequires:	 qwt-qt5-devel
%else
BuildRequires:  %{!?_debbuild:qwt6-qt5-devel}      %{?_debbuild:libqwt-qt5-dev}
%endif
%if 0%{?is_opensuse}
BuildRequires:  libQt5OpenGL-devel
%else
BuildRequires:  %{!?_debbuild:qt5-qtsvg-devel}      %{?_debbuild:libqt5opengl5-dev}
%endif

Requires:       %{name}-libs

%description
C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        gui
Summary:	       GUI Ion transport simulation in materials

Requires:       %{name}-libs

%description    gui
GUI C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        libs
Summary:	       Libraries for ion transport simulation in materials

%description    libs
Libraries for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        devel
Summary:	       Development files for ion transport simulation in materials

Requires:       %{name}-libs

%description    devel
Development files for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        tests
Summary:	       Test files for ion transport simulation in materials
BuildArch:      noarch

%if "%{_vendor}" == "debbuild"
Requires:       %{name} | %{name}-gui
%else
Requires:       ( %{name} or %{name}-gui )
%endif

%description    tests
Test files for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package -n     %{__python_package}-opentrim
Summary:	       Python bindings for OpenTRIM

BuildRequires:  %{!?_debbuild:%{__python_package}-devel}      %{?_debbuild:%{__python_package}-dev}

%if 0%{?is_opensuse}
BuildRequires:  %{__python_package}-pybind11-devel
%else
BuildRequires:  %{!?_debbuild:pybind11-devel}      %{?_debbuild:pybind11-dev}
%endif

Requires:       %{name}-libs = %{version}
Requires:       %{__python_package}-numpy

%description -n %{__python_package}-opentrim
Python 3 bindings for the OpenTRIM Monte-Carlo ion transport simulator:
configure, run and evaluate simulations from Python via the Config, Driver
and Info classes.
The bindings are built in-tree against the freshly built library and installed
into %{__my_python_site} (pass -DOPENTRIM_PYTHON_INSTALL_DIR= to override if
that macro is not defined on the target).

###############################################################################################################################

%if "%{_vendor}" != "debbuild" && "%{debug_build}" == "1"
%debug_package
%else
%global         debug_package               %{nil}
%endif

###############################################################################################################################

%prep
%setup -q -n	 %{name}
mkdir -p external
tar -zxf %{SOURCE11} -C external
tar -zxf %{SOURCE12} -C external
tar -zxf %{SOURCE13} -C external
tar -zxf %{SOURCE14} -C external
tar -zxf %{SOURCE15} -C external
tar -zxf %{SOURCE17} -C external
tar -zxf %{SOURCE18} -C external
tar -zxf %{SOURCE19} -C external

%build
%cmake \
   -DCMAKE_C_COMPILER=gcc%{?__gcc_min_version:-%{__gcc_min_version}} \
   -DCMAKE_CXX_COMPILER=g++%{?__gcc_min_version:-%{__gcc_min_version}} \
   -DPACKAGE_BUILD=ON \
   -DCMAKE_BUILD_TYPE=Release \
   -DOPENTRIM_BUILD_GUI=ON \
   -DOPENTRIM_BUILD_PYTHON=ON \
   -DOPENTRIM_PYTHON_INSTALL_DIR=%{__my_python_site} \
   %{nil}

%cmake_build

%install
%cmake_install

strip --strip-unneeded %{buildroot}%{_bindir}/%{name}*
strip --strip-unneeded %{buildroot}%{_libdir}/lib*.so
strip --strip-unneeded %{buildroot}%{__my_python_site}/%{name}/_opentrim_core*.so || :

install -d %{buildroot}/%{_datadir}/%{name}/tests
install -d %{buildroot}/%{_datadir}/%{name}/examples

cp -r test/%{name}/* %{buildroot}/%{_datadir}/%{name}/tests/
cp -r examples/*     %{buildroot}/%{_datadir}/%{name}/examples/

#%%check
#%%ctest "-V" "-j1"

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%{_bindir}/%{name}
%dir %{_datadir}/%{name}
%dir %{_datadir}/%{name}/examples
%{_datadir}/%{name}/examples

%files gui
%{_bindir}/%{name}-gui

%files libs
%{_libdir}/lib*.so

%files devel
%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*.h
%dir %{_libdir}/cmake/%{name}
%{_libdir}/cmake/%{name}/*.cmake

%files tests
%dir %{_datadir}/%{name}/tests
%{_datadir}/%{name}/tests

%files -n %{__python_package}-opentrim
%{__my_python_site}/opentrim/

%changelog
