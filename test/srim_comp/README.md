# Comparison of SRIM/OpenTRIM damage estimations

We compare the damage estimated by SRIM and OpenTRIM in a number of test cases listed in Li et al. 2023 and Agarwal et al. 2021

## Running the test simulations

### SRIM 

SRIM is run in 2 different modes:
- Quick Cascade (QC)
- Full cascade (FC) with $E_{min} = E_d$

In the 2nd case we follow this procedure (from Lin2023):
- Run SRIM-FC for 1 ion and save 
- Open `SRIM Restore/TDATA.sav` and below the line `Lowest E,  Ed(min)  (eV)` set the first number equal to $E_d$. Essentially, this sets the lowest energy of moving ions.  
- Select `continue saved run` and run the number of ions you want 

Damage is obtained from the VACANCY.txt files
- In QC we add the 2 columns (V from ions + V from recoils)
- In FC we take only the 2nd column (Target atom vacancies)

### OpenTRIM 

OpenTRIM is run with exactly the same geometry and material composition. 
The `/tally/damage_events/Vacancies` dataset corresponds to the FC data and the `/tally/pka_damage/Vnrt_LSS` dataset corresponds to QC.

## Projectiles

- [ ] 1 MeV H
- [ ] 1 MeV He
- [ ] 3 MeV Al
- [X] 5 MeV Fe
- [ ] 10 MeV Au

## Targets

15 targets with Z from 3 (Li) to 92 (U). See the file [targets.csv](./targets.csv)

## Results

![](../octave/srim_comp_5MeVFe.png)