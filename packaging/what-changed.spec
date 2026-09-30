Name:           what-changed
Version:        1.0.0
Release:        1%{?dist}
Summary:        Privacy-conscious Linux system change detector
License:        MIT
Vendor:         GuhsHosting
URL:            https://github.com/guhshosting/what-changed
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  gcc-c++

%description
Capture curated Linux system snapshots and compare observed changes without
modifying system configuration or asserting causality.

%prep
%autosetup

%build
%cmake
%cmake_build

%install
%cmake_install

%files
%license LICENSE
%doc README.md docs/architecture.md docs/privacy.md docs/snapshot-format.md
%{_bindir}/what-changed
%{_mandir}/man1/what-changed.1*

%changelog
* Wed Sep 16 2026 GuhsHosting <guhshosting@users.noreply.github.com> - 1.0.0-1
- Initial release
