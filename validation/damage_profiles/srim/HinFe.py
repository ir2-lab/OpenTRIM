# %%
import sys
import time # for timing
from srim import TRIM, SR, Ion, Layer, Target
from srim.output import Results, Range
import logging

# default simulation data
n_ions = 10 # ion histories to run
srim_mode = 1 # 1-QC, 2-FC, 3-MonoLayer
ionE0 = 1e6 # ion energy
target_thickness = 1000 # nm

if len(sys.argv) > 1:
    n_ions = int(sys.argv[1])

if len(sys.argv) > 2:
    srim_mode = int(sys.argv[2])

if len(sys.argv) > 3:
    ionE0 = int(sys.argv[3])

if len(sys.argv) > 4:
    target_thickness = int(sys.argv[4])

# Specify the directory of SRIM.exe
# 1. for docker
srim_executable_directory = '/tmp/srim'
output_folder = '/tmp/output'
log_file = '/opt/pysrim/HinFe.log'

logging.basicConfig(filename=log_file, level=logging.DEBUG)
print(f"Nions: {n_ions}, mode: {srim_mode}, E0: {ionE0}, t: {target_thickness}")
logging.info(f"Nions: {n_ions}, mode: {srim_mode}, E0: {ionE0}, t: {target_thickness}")

# %%
ion = Ion('H', energy=ionE0, mass=1.00784)

# Make card for simulation [Energy] = eV , desity in Angstr.
layer = Layer({
    'Fe': {
        'stoich': 1.0,
        'E_d': 40,
        'lattice': 3.0,
        'surface': 40.0,
        'mass': 55.847
    }}, density=7.8658, width=target_thickness*10)

target = Target([layer])

# Initialize a TRIM calculation with given target and ion for 25 ions, quick calculation
trim = TRIM(target, ion, number_ions=n_ions, 
            calculation=srim_mode, # 2: FC, 3: monolayer steps
            collisions=1, # store collisions, only ions
            plot_mode=5) # plot_mode=0 (ions), 5 (no graphics)

# Run TRIM & get results
elapsed_time = time.process_time()
results = trim.run(srim_executable_directory)
elapsed_time = time.process_time() - elapsed_time
print(f"Elapsed time: {elapsed_time:0.4f} s, {(n_ions/elapsed_time):0.2f} ions/s")
logging.info(f"Elapsed time: {elapsed_time:0.4f} s, {(n_ions/elapsed_time):0.2f} ions/s")

# Transfer output files the out folder
TRIM.copy_output_files(srim_executable_directory, output_folder)

# %%
