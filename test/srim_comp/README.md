# Comparison of SRIM/OpenTRIM damage estimations

We compare the damage estimated by SRIM and OpenTRIM in a number of test cases listed in Li et al. 2023 and Agarwal et al. 2021

## Running the test simulations

The tests are run with 10000 ion histories.

### SRIM

SRIM is run in 2 different modes:

- Quick Cascade (QC)
- Full cascade (FC) with $E\_{min} = E_d$

The file [RUN_SRIM_QC.md](RUN_SRIM_QC.md) details the procedure to run the QC simulations.

In the 2nd case we follow this procedure (from Lin2023):

- Run SRIM-FC for 1 ion and save
- Open `SRIM Restore/TDATA.sav` and below the line `Lowest E,  Ed(min)  (eV)` set the first number equal to $E_d$. Essentially, this sets the lowest energy of moving ions.
- Select `continue saved run` and run the number of ions you want

The file [RUN_SRIM_FC.md](RUN_SRIM_FC.md) details the procedure to run the FC simulations.

Damage is obtained from the VACANCY.txt files

- In QC we add the 2 columns (V from ions + V from recoils)
- In FC we take only the 2nd column (Target atom vacancies)

### OpenTRIM

OpenTRIM is run with exactly the same geometry and material composition.
The `/tally/damage_events/Vacancies` dataset corresponds to the FC data and the `/tally/pka_damage/Vnrt_LSS` dataset corresponds to QC.

## Projectiles

5 Projectiles, data in [projectiles.csv](./projectiles.csv)

- 1 MeV H 
  - [x] QC
  - [x] FC
- 1 MeV He 
  - [x] QC
  - [x] FC
- 3 MeV Al 
  - [x] QC
  - [x] FC
- 5 MeV Fe 
  - [x] QC
  - [x] FC
- 10 MeV Au 
  - [x] QC
  - [x] FC

## Targets

15 targets with Z from 3 (Li) to 92 (U). See the file [targets.csv](./targets.csv)

## Results

[compare_opentrim_srim.pdf](./compare_opentrim_srim.pdf)
