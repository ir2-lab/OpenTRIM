## 1. Configuration File
| **Ion Distribution** | **Explanation** | 
|:--------:|:--------:|
|  0  | Completely random distribution of entry positions of the ion on the x = 0 plane. The other four parameters are ignored (and can be omitted).   | 
|  1  | All ions enter at the center of the target and at x = 0 (like in TRIM). The other four parameters are ignored (and can be omitted).   | 
|  2  | All ions enter at the defined (x, y, z) positions. The position must be specified by *enter_x . . . enter_z* in units of nm |
| 3 | Like 2, but the ions are spread randomly around the entry point with fixed x = 0. beam_spread specifies how much the y and z position is spread around the defined entry point (in units of nm) |
| 4 | All ions start at the bulk center position in the simulation volume |
| | |


![Ion Distribution](manual_iradina/distribution.png)



| **Simulation_type** | **Explanation** | 
|:--------:|:--------:|
|  0  | Full simulation including detailed following of all recoils in collision cascades and detailed calculation of damage. (Similar to TRIM’s “Detailed calculation with full damage cascades”). | 
|  3  | Ions distribution only. Recoils are not followed, damage is not calculated. Faster. This does only work when transport_type is set 1. Otherweise iradina always considers recoils.   | 
|  4  | Kinchin-Pease quick calculation of damage. Note: with this option iradina assumes that after each collision, the target consists purely of atoms of the same type as the current recoil. This seems to be the same as what TRIM is doing.|
| 5 | Kinchin-Pease quick calculation of damage but based on the average target material composition. This is different from what TRIM seems to be doing but follows the SRIM book (page 7-28). |
| other numbers | Undefined behaviour of program. |
| | |

**Options 4 and 5 should only be used together with *flight_length_type=3* and also with *transport_type=1***

![Simulation_table](manual_iradina/simulation.png)
![Simulation_table2](manual_iradina/simulation2.png)


| **flight_length_type** | **Explanation** | 
|:--------:|:--------:|
|  0  | The flight length is varied randomly according to a Poisson distribution with an average flight length according to the mean interatomic distance. | 
|  1  | The flight length is constant and always corresponds to the mean interatomic distance.  | 
|  2  | The flight length is constant and always exactly the number specified by the flight_length_constant parameter (in units of nm).|
| 3 | The flight length is calculated as the free flight path in SRIM. This option should be used if Kinchin-Pease quick calculation of damage is selected. Note that this option must not be used together with *transport_type=0*. |
| | |

![flight](manual_iradina/flight_length.png)

- Simulation time scales approximately with inverse flight length. However, flight lengths far larger than the interatomic spacing will generate mostly inaccurate results. **Setting this option to zero is
recommended (unless using Kinchin-Pease quick calculation of damage)**, but this is something you might
think about..

- The transport algorithm is selected by setting **transport_type**. There are two different versions implemented. Using “0” is more accurate (especially important for sputter calculations!). Transport
mode “1” is much faster (more similar to the computing flow in Corteo), yields similar ion and damage
distributions but much worse sputter yields!  

| **scattering_calculation** | **explanation** |
|:--------:|:--------:|
| 0  | iradina uses fast database method for calculation of the scattering angle |
| 1 | iradina uses the MAGIC algorithm. This only works when *transport_type* is set to “0”. |
| | |


- Setting the parameter *store_energy_deposit* to 1 instructs the program to create and store
two arrays, that sum up electronic and nuclear energy loss in each cell.


| **normalize_output** | **explanation** |
|:--------:|:--------:|
| 0  | radina will not normalize the output at all, meaning that the integer values for the counters (ions, defects, ...) of each cell are stored. |
| 1 | iradina will store output results in units of *(1/cm$^3$ ) per (ions/cm$^2$ )*. This allows the direct calculation of the concentration of implanted ions or some defect type for a specific fluence by simply multipling the results with this fluence. Note however, this is a little different from TRIM: iradina calculates ions/cm$^2$ by assuming that the reference plane in cm$^2$ is perpendicular to the ion beam. In TRIM, cm$^2$ corresponds to sample surface.|
| 2 | TRIM like behavior is activated |
|||

## 4. Output file

![output1](manual_iradina/output1.png)
![output2](manual_iradina/output2.png)
![output3](manual_iradina/output3.png)