#!/usr/bin/env python3
# OpenTRIM "3 MeV Xe on UO2" example
#
# This script shows how to run the "3 MeV Xe on UO2" example from OpenTRIM's GUI using Python.
#
# Basic steps:
# - JSON configuration is prepared in Python and saved to a file
# - `opentrim` command-line executable is called with the JSON input
# - HDF5 output file is opened with `h5py`
# - Data plots are created with `matplotlib`
#
# It is assumed that `opentrim` is available on your `PATH`.
# If it is not, set `exe` to the full path (example in the comment below).

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
# Using OpenTRIM's `-t` CLI option
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
# Main options for "3 MeV Xe on UO2" example:
# - Ion beam: 3 MeV Xe
# - Target material: UO2
# - Target size: 1200 nm cube
# - Flight path: "Variable"

Config["Transport"]["flight_path_type"] = "Variable"

Config["IonBeam"]["ion"] = {
    "symbol": "Xe",
    "atomic_mass": 131.904,
    "atomic_number": 54
}
Config["IonBeam"]["energy_distribution"]["center"] = 3e6
Config["IonBeam"]["spatial_distribution"]["center"] = [0, 600, 600]

Config["Target"]["size"] = [1200, 1200, 1200]
Config["Target"]["cell_count"] = [100, 1, 1]
Config["Target"]["periodic_bc"] = [0, 1, 1]

Config["Target"]["materials"][0]["id"] = "UO2"
Config["Target"]["materials"][0]["density"] = 10.97
Config["Target"]["materials"][0]["composition"] = [
    {"element": {"symbol": "U"}, "X": 1, "Ed": 40, "El": 4, "Es": 4, "Er": 40},
    {"element": {"symbol": "O"}, "X": 2, "Ed": 20, "El": 2, "Es": 2, "Er": 20}
]

Config["Target"]["regions"][0]["id"] = "R1"
Config["Target"]["regions"][0]["material_id"] = "UO2"
Config["Target"]["regions"][0]["size"] = [1200, 1200, 1200]

Config["Simulation"]["simulation_type"] = "FullCascade"
Config["Simulation"]["screening_type"] = "ZBL"
Config["Simulation"]["nrt_calculation"] = "NRT_average"

Config["Output"]["title"] = "3MeV Xe on UO2 example"
Config["Output"]["store_pka_events"] = True
Config["Output"]["store_dedx"] = True
Config["Output"]["store_exit_events"] = True

Config["Run"]["threads"] = 0
Config["Run"]["max_no_ions"] = 4000

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
    x = np.asarray(f["/target/grid/X"][0:100])
    v = np.asarray(f["/tally/damage_events/Vacancies"][1:3, 0:100])  # (2,100,1,1)
    v = np.squeeze(v)  # -> (2,100)
    Nuo2 = f["/target/materials/atomic_density"][()]*1e21

# 6. Plot vacancy profiles for U and O
plt.figure()
plt.plot(x, v[0, :], label="U vacancies")
plt.plot(x, v[1, :], label="O vacancies")
plt.xlabel("depth (nm)")
plt.ylabel("Vacancies / ion")
plt.title("3 MeV Xe on UO2: Vacancies")
plt.legend()
plt.tight_layout()
plt.show()