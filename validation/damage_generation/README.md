## Comparison to SRIM damage estimations

We compare the damage estimated by SRIM and OpenTRIM in a matrix of test cases listed in Li et al. 2023 and Agarwal et al. 2021.

The matrix comprises
- 5 Projectiles, 1MeV H, 1MeV He, 3MeV Al, 5MeV Fe, 10MeV Au. Data in [projectiles.csv](./projectiles.csv)
- 15 targets with Z from 3 (Li) to 92 (U). See the file [targets.csv](./targets.csv)
- total: 75 cases. 

The simulations are performed in Quick-Cascade (QC) and Full-Cascade (FC) mode for 10000 ion histories.

## Results

The results can be seen in the following document.
[compare_opentrim_srim.pdf](./compare_opentrim_srim.pdf)

The graphs show relative difference in vacancy generation predicted by the two codes.

In general we have:
- QC mode: 
  - difference up to 1-3%, in all projectiles except 1MeV H
  - 1MeV H projectile shows generally larger discrepancies in QC mode (up to 7%). May be due to light atom approximation in SRIM
  - [ ] **TODO**. Run 1MeV H also in SRIM monolayer mode
- FC mode: 
  - difference up to 5%, except for Cu target 
  - **Cu FC mode:**, OpenTRIM damage prediction is significantly higher, reaching up to 20%. Reason unknown.

## Files

- `OTRIM_QC.dat`, `OTRIM_FC.dat`: Processed OpenTRIM vacancy generation data
- `ERROR_QC.dat`, `ERROR_FC.dat`: Processed OpenTRIM errors
- `SRIM_QC.dat`, `SRIM_FC.dat`: Processed SRIM vacancy generation data
- All `.dat` files are 15 rows x 5 columns ascii tables (target x projectile)
- The raw data can be downloaded from [here](https://fusion.ipta.demokritos.gr/nextcloud/index.php/s/g2wESYDnXNLSsTS)  
- `projectiles.csv`, `targets.csv`: all needed data values
- `compare_opentrim_srim.m`: octave script to generate final comparison plot
- `RUN_SRIM_QC.md`: instructions on how to run SRIM in QC mode
- `RUN_SRIM_FC.md`: instructions on how to run SRIM in FC mode

## Details on running the test cases

### SRIM

SRIM is run in 2 different modes:

- Quick Cascade (QC)
- Full cascade (FC) with $E_{min} = E_d$

The file [RUN_SRIM_QC.md](RUN_SRIM_QC.md) details the procedure to run the QC simulations.

In the FC case we follow the procedure described in Lin2023 to set $E_{min}$:

- Run SRIM-FC for 1 ion and save
- Open `SRIM Restore/TDATA.sav` and below the line `Lowest E,  Ed(min)  (eV)` set the first number equal to $E_d$. Essentially, this sets the lowest energy of moving ions.
- Select `continue saved run` and run the number of ions you want

The file [RUN_SRIM_FC.md](RUN_SRIM_FC.md) details the procedure to run the FC simulations.

Damage is obtained from the VACANCY.txt files

- In QC we add the 2 columns (V from ions + V from recoils)
- In FC we take only the 2nd column (Target atom vacancies)

### OpenTRIM details

OpenTRIM is run with exactly the same geometry and material composition.

There is no need to run the simulation twice for QC/FC, since OpenTRIM always stores the LSS/NRT vacancy estimation that corresponds to SRIM QC.

From the output HDF5 file we load 2 datasets:
- `/tally/damage_events/Vacancies` corresponds to the FC data
- `/tally/pka_damage/Vnrt_LSS` corresponds to QC data.


