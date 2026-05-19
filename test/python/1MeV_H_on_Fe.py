#!/usr/bin/env python3
# OpenTRIM "1 MeV H in Fe" example
#
# This script shows how to run the "1 MeV H in Fe" example from OpenTRIM's GUI using Python.
#
# Basic steps:
# - JSON configuration is prepared in Python and saved to a file
# - `opentrim` command-line executable is called with the JSON input
# - HDF5 output file is opened with `h5py`
# - Data plots are created with `matplotlib`
#
# It is assumed that `opentrim` is available on your `PATH`.

# 1. Load required Python modules

from pathlib import Path
import json
import subprocess
import tempfile
from typing import Any, Dict

import h5py
import numpy as np
import matplotlib
matplotlib.use("TkAgg") #for displaying plot in WSL using X server
import matplotlib.pyplot as plt

# 2. Generate the standard json configuration template.
exe = "opentrim"  # replace with full path if necessary, e.g. "/home/[user]/bin/opentrim"
cmd = [exe, "-t"]

template_run = subprocess.run(
    cmd,
    text=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    timeout=20,
)

template_run.check_returncode()
Config: Dict[str, Any] = json.loads(template_run.stdout)

# 3. Edit the configuration
# Main options for "1 MeV H in Fe" example:
# - Ion beam: 1 MeV H+ (default - no change needed)
# - Target material: Fe (default - no change needed)
# - Target size: 10 μm square
# - Flight path: "Variable"

Config["Target"]["size"] = [10000.0, 10000.0, 10000.0]
Config["Target"]["cell_count"] = [100, 1, 1]
Config["Target"]["regions"][0]["size"] = [10000.0, 10000.0, 10000.0]
Config["IonBeam"]["spatial_distribution"]["center"] = [0.0, 5000.0, 5000.0]
Config["Transport"]["flight_path_type"] = "Variable"
Config["Output"]["title"] = "1 MeV H on Fe example"
Config["Run"]["threads"] = 0
Config["Run"]["max_no_ions"] = 20000

# 4. Run `opentrim` in a temp folder
tmpdir = tempfile.TemporaryDirectory(prefix="opentrim_")
tmppath = Path(tmpdir.name)

config_path = tmppath / "config.json"
config_path.write_text(json.dumps(Config, indent=4) + "\n", encoding="utf-8")

cmd = [exe, "-f", "config.json", "-o", "result", "-s", "42"]
result = subprocess.run(
    cmd,
    cwd=tmppath,
    text=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    stdin=subprocess.DEVNULL,
    timeout=120,
)

if result.returncode:
    print("stderr:\n", result.stderr)

# 5. Read data from HDF5 output file
h5_path = tmppath / "result.h5"
if not h5_path.exists():
    raise FileNotFoundError(f"Expected output file not found: {h5_path}")

with h5py.File(h5_path, "r") as f:
    x = np.asarray(f["/target/grid/X"][0:100])  # shape (101,)
    v = np.asarray(f["/tally/damage_events/Vacancies"][1, 0:100, 0, 0])
    Nfe = f["/target/materials/atomic_density"][()]*1e21

# 6. Plot displacement damage profile
F0 = 1e16  # ion flux
dx = (x[1]-x[0]) * 1e-7  # x grid step in cm

plt.figure()
plt.plot(x*1e-3, v*(F0/dx/Nfe))
plt.xlabel("depth (μm)")
plt.ylabel(r"Dpa $\div$ ion fluence [10$^{16}$ cm$^{-2}$]")
plt.title("1 MeV H in Fe: Displacement Damage Profile")
plt.tight_layout()
plt.show()