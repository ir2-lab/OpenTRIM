#!/usr/bin/env bash
# install_oct_package.sh — build and install the opentrim Octave package.
#
# Usage:
#   bash dist/octave/install_oct_package.sh <version>
#   e.g. bash dist/octave/install_oct_package.sh 1.1.6
#
# Builds opentrim-octave-<version>.tar.gz via make_oct_package.sh and then
# installs it into Octave with `pkg install`.

set -euo pipefail

# --------------------------------------------------------------------------
# Arguments
# --------------------------------------------------------------------------
if [ $# -lt 1 ]; then
    echo "Usage: $0 <version>" >&2
    echo "  e.g. $0 1.1.6" >&2
    exit 1
fi

VERSION="$1"

# --------------------------------------------------------------------------
# Build the package tarball
# --------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

"${SCRIPT_DIR}/make_oct_package.sh" "${VERSION}"

PACKAGE_FILE="${PWD}/opentrim-octave-${VERSION}.tar.gz"

if [ ! -f "${PACKAGE_FILE}" ]; then
    echo "ERROR: expected package file not found at ${PACKAGE_FILE}" >&2
    exit 1
fi

# --------------------------------------------------------------------------
# Install into Octave
# --------------------------------------------------------------------------
octave --eval "pkg install -verbose '${PACKAGE_FILE}'"
