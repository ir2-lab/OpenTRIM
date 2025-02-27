%define debug_package %{nil}

Name:          opentrim
Version:	      0
Release:	      0
Summary:	      Ion transport simulation in materials
License:	      GPL-3.0-or-later
Group:         Productivity/Scientific/Physics
Url:		      http://geant4.web.cern.ch/geant4/support/source/

Source0:	      %{name}.tar.gz

Source11:      ext1.tar.gz
Source12:      ext2.tar.gz
Source13:      ext3.tar.gz
Source14:      ext4.tar.gz
Source15:      ext5.tar.gz
Source16:      ext6.tar.gz

BuildRequires:	cmake >= 3.12
BuildRequires:	( gcc-c++ >= 9.0 or gcc9-c++ )

Requires:      %{name}-libs

%description
C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package       gui
Summary:	      GUI Ion transport simulation in materials
Group:         Productivity/Scientific/Physics

BuildRequires:	qwt6-qt5-devel

Requires:      %{name}-libs

%description   gui
GUI C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package       libs
Summary:	      Libraries for ion transport simulation in materials
Group:         Productivity/Scientific/Physics

BuildRequires:	eigen3-devel >= 3.4
BuildRequires:	hdf5-devel

%description   libs
Libraries for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package       devel
Summary:	      Development files for ion transport simulation in materials
Group:         Development/Languages/C and C++
BuildArch:     noarch

Requires:      %{name}-libs

%description   devel
Development files for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%package       tests
Summary:	      Test files for ion transport simulation in materials
Group:         Development/Languages/C and C++
BuildArch:     noarch

Requires:      ( %{name} or %{name}-gui )

%description   tests
Test files for C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on the calculation of material damage.

%prep
%setup -q -n	%{name}
mkdir -p external
tar -zxf %{SOURCE11} -C external
tar -zxf %{SOURCE12} -C external
tar -zxf %{SOURCE13} -C external
tar -zxf %{SOURCE14} -C external
tar -zxf %{SOURCE15} -C external
tar -zxf %{SOURCE16} -C external

%build
%cmake \
%if %(g++ -dumpversion) < 9
   -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 \
%endif
   -DPACKAGE_BUILD=ON \
   -DCMAKE_BUILD_TYPE=Release \
   %{nil}

%cmake_build

%install
%cmake_install
install -d %{buildroot}/%{_datadir}/%{name}/tests
cp -r test/%{name}/* %{buildroot}/%{_datadir}/%{name}/tests/

%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%files
%{_bindir}/%{name}

%files gui
%{_bindir}/%{name}-gui

%files libs
%{_libdir}/lib*

%files devel
/usr/include/*

%files tests
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/tests

%changelog
