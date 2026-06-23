#!/usr/bin/env bash
# make_oct_package.sh — assemble the opentrim Octave package tarball.
#
# Usage:
#   bash dist/make_oct_package.sh <version>
#   e.g. bash dist/make_oct_package.sh 1.1.6
#
# Produces: opentrim-octave-<version>.tar.gz in the current directory.
# The archive contains a single top-level 'opentrim/' directory as
# required by Octave's pkg install mechanism.

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
TODAY="$(date +%Y-%m-%d)"

# --------------------------------------------------------------------------
# Locate repository root (script lives in dist/)
# --------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BINDINGS_DIR="${REPO_ROOT}/bindings/octave"
OUTPUT_FILE="${PWD}/opentrim-octave-${VERSION}.tar.gz"

if [ ! -d "${BINDINGS_DIR}" ]; then
    echo "ERROR: bindings/octave directory not found at ${BINDINGS_DIR}" >&2
    exit 1
fi

# --------------------------------------------------------------------------
# Staging area (cleaned up on exit)
# --------------------------------------------------------------------------
STAGING="$(mktemp -d /tmp/opentrim_oct_pkg_XXXXXX)"
trap 'rm -rf "${STAGING}"' EXIT

PKG_DIR="${STAGING}/opentrim"
mkdir -p "${PKG_DIR}"

# --------------------------------------------------------------------------
# Copy package files
# --------------------------------------------------------------------------
cp -a "${BINDINGS_DIR}/." "${PKG_DIR}/"

# --------------------------------------------------------------------------
# Patch DESCRIPTION in staging area (never touch the source tree)
# --------------------------------------------------------------------------
DESCRIPTION="${PKG_DIR}/DESCRIPTION"

# Update Version: and Date: lines
sed -i \
    -e "s|^Version:.*|Version: ${VERSION}|" \
    -e "s|^Date:.*|Date: ${TODAY}|" \
    "${DESCRIPTION}"

echo "Patched DESCRIPTION: Version=${VERSION}, Date=${TODAY}"

# --------------------------------------------------------------------------
# Create tarball
# --------------------------------------------------------------------------
tar -czf "${OUTPUT_FILE}" -C "${STAGING}" opentrim

echo "Created: ${OUTPUT_FILE}"
