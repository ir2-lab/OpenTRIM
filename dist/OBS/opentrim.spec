%global         debug_package     %{nil}
%global         __strip           /bin/true

Name:           opentrim
Version:	       0
Release:	       0
Summary:	       Ion transport simulation in materials
License:	       GPL-3.0-or-later
Url:		       https://github.com/ir2-lab/OpenTRIM.git

%if "%{_vendor}" == "debbuild"
Packager:       M. Axiotis <psaxioti@gmail.com>
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
BuildRequires:	 qtdatabrowser-devel
BuildRequires:	 qmatplotwidget-devel

%if "%{_vendor}" == "debbuild"

   %if 0%{?ubuntu_version} >= 2204 || 0%{?debian_version} >= 1100
BuildRequires:  debhelper-compat = 13
   %else
BuildRequires:  debhelper-compat = 12
   %endif
BuildRequires:  debbuild-macros

BuildRequires:  g++ >= 11.0
BuildRequires:  qtbase5-dev
BuildRequires:  libqwt-qt5-dev
BuildRequires:  libqt5svg5-dev
BuildRequires:	 libeigen3-dev >= 3.4
BuildRequires:	 libhdf5-dev

%else

   %if 0%{?sle_version} == 150600 && 0%{?is_opensuse}
BuildRequires:  gcc11-c++
   %else
BuildRequires:  gcc-c++ >= 11.0
   %endif
   %if 0%{?centos_version} && 0%{?centos_version} == 1000
BuildRequires:	 qwt-qt5-devel
   %else
BuildRequires:	 qwt6-qt5-devel
   %endif
BuildRequires:	 eigen3-devel >= 3.4
BuildRequires:	 hdf5-devel

%endif

#Requires:       %{name}-libs

%description
C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        gui
Summary:	       GUI Ion transport simulation in materials

#Requires:       %{name}-libs

%description    gui
GUI C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        libs
Summary:	       Libraries for ion transport simulation in materials

%description    libs
Libraries for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package        devel
Summary:	       Development files for ion transport simulation in materials

#Requires:       %{name}-libs

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
%if %(g++ -dumpversion) < 11
   -DCMAKE_C_COMPILER=gcc-11 \
   -DCMAKE_CXX_COMPILER=g++-11 \
%endif
   -DPACKAGE_BUILD=ON \
   -DCMAKE_BUILD_TYPE=Release \
   %{nil}

%cmake_build

%install
%cmake_install

strip --strip-unneeded %{buildroot}%{_bindir}/%{name}*
strip --strip-unneeded %{buildroot}%{_libdir}/lib*.so

install -d %{buildroot}/%{_datadir}/%{name}/tests

cp -r test/%{name}/* %{buildroot}/%{_datadir}/%{name}/tests/

%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%files
%{_bindir}/%{name}

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
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/tests

%changelog
