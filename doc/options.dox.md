
## The JSON configuration file {#json_config}

All configuration parameters for a simulation are coded in a [JSON](https://www.json.org/json-el.html) formatted string. 

This can be loaded directly from a file to the \ref cliapp "opentrim cli program" with the following command:

    opentrim -f config.json

where `config.json` is a file containing the JSON configuration.

In a C++ program one can use the \ref mcdriver::options class which offers
a parseJSON() function to parse and validate the options.
                        
The easiest way to get started is to generate a template with all default options by running
                                        
    opentrim -t > template.json
                        
and make changes to the new template file.

@note `opentrim` accepts comments in JSON 

The JSON config string has the following self-explanatory structure shown bellow.
Click on any of the options to see more information.

> <br>
{<br>
&emsp;&emsp;\ref _Simulation "\"Simulation\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_simulation_type "\"simulation_type\"": "FullCascade",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_screening_type "\"screening_type\"": "ZBL",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_eloss_calculation "\"eloss_calculation\"": "EnergyLoss",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_straggling_model "\"straggling_model\"": "BohrStraggling",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_nrt_calculation "\"nrt_calculation\"": "NRT_element",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_intra_cascade_recombination "\"intra_cascade_recombination\"": false<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Transport "\"Transport\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_min_energy "\"min_energy\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_flight_path_type "\"flight_path_type\"": "AtomicSpacing",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_flight_path_const "\"flight_path_const\"": 0.1,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_max_rel_eloss "\"max_rel_eloss\"": 0.05,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_min_recoil_energy "\"min_recoil_energy\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_max_mfp "\"max_mfp\"": 1e+30,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_allow_sub_ml_scattering "\"allow_sub_ml_scattering\"": false<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _IonBeam "\"IonBeam\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_ion "\"ion\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_ion_symbol "\"symbol\"": "H",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_ion_atomic_number "\"atomic_number\"": 1,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_ion_atomic_mass "\"atomic_mass\"": 1.007825<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_energy_distribution "\"energy_distribution\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_energy_distribution_type "\"type\"": "SingleValue",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_energy_distribution_center "\"center\"": 1e+06,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_energy_distribution_fwhm "\"fwhm\"": 1.0<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_spatial_distribution "\"spatial_distribution\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_spatial_distribution_geometry "\"geometry\"": "Surface",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_spatial_distribution_type "\"type\"": "SingleValue",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_spatial_distribution_center "\"center\"": [0.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_spatial_distribution_fwhm "\"fwhm\"": 1.0<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_angular_distribution "\"angular_distribution\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_angular_distribution_type "\"type\"": "SingleValue",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_angular_distribution_center "\"center\"": [1.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _IonBeam_angular_distribution_fwhm "\"fwhm\"": 1.0<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Target "\"Target\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_size "\"size\"": [100.0,100.0,100.0],<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_origin "\"origin\"": [0.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_cell_count "\"cell_count\"": [1,1,1],<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_periodic_bc "\"periodic_bc\"": [0,1,1],<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_materials "\"materials\"": {<br>
<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_regions "\"regions\"": {<br>
<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Output "\"Output\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_title "\"title\"": "Ion Simulation",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_storage_interval "\"storage_interval\"": 1000,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_exit_events "\"store_exit_events\"": 0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_pka_events "\"store_pka_events\"": 0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_dedx "\"store_dedx\"": 1<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Driver "\"Driver\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Driver_max_no_ions "\"max_no_ions\"": 100,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Driver_threads "\"threads\"": 1,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Driver_seed "\"seed\"": 123456789<br>
&emsp;&emsp;}<br>
}<br>
<br>





<table>
<caption>Detailed Description</caption>
<tr><th colspan="2">\anchor _Simulation Simulation<tr><td>Type <td>Option group
<tr><td>Description <td>General simulation options
<tr><th colspan="2">\anchor _Simulation_simulation_type Simulation.simulation_type<tr><td>Type <td>Enumerator
<tr><td>Values<td> FullCascade | IonsOnly | CascadesOnly
<tr><td>Default Value<td>"FullCascade"<tr><td>Description <td>Define the type of simulation.
<br>
- FullCascade: Simulate full damage cascades, follow recoils<br>
- IonsOnly: Simulate only ions, do not follow recoils<br>
- CascadesOnly: Simulate recoil cascades<br>
<tr><th colspan="2">\anchor _Simulation_screening_type Simulation.screening_type<tr><td>Type <td>Enumerator
<tr><td>Values<td> None | LenzJensen | KrC | Moliere | ZBL | ZBL_MAGIC
<tr><td>Default Value<td>"ZBL"<tr><td>Description <td>Define the type of screening potential.
<br>
- None: Unscreened Coulomb potential<br>
- LenzJensen: Lenz-Jensen screening<br>
- KrC: Kr-C screening<br>
- Moliere - Moliere screening<br>
- ZBL - Ziegler-Biersack-Littmark universal screening<br>
- ZBL_MAGIC - ZBL computed with MAGIC formula<br>
  <br>
For Unscreened Coulomb and ZBL_MAGIC the scattering is calculated analytically.<br>
In all other cases interpolation tables are used.<br>
<tr><th colspan="2">\anchor _Simulation_eloss_calculation Simulation.eloss_calculation<tr><td>Type <td>Enumerator
<tr><td>Values<td> EnergyLossOff | EnergyLoss | EnergyLossAndStraggling
<tr><td>Default Value<td>"EnergyLoss"<tr><td>Description <td>Setting for electronic energy loss calculation.
<br>
- EnergyLossOff: No energy loss calculation<br>
- EnergyLoss: Only energy loss is calculated<br>
- EnergyLossAndStraggling: Both energy loss and straggling are calculated<br>
<tr><th colspan="2">\anchor _Simulation_straggling_model Simulation.straggling_model<tr><td>Type <td>Enumerator
<tr><td>Values<td> BohrStraggling | ChuStraggling | YangStraggling
<tr><td>Default Value<td>"BohrStraggling"<tr><td>Description <td>Model used for electronic straggling calculations.
<tr><th colspan="2">\anchor _Simulation_nrt_calculation Simulation.nrt_calculation<tr><td>Type <td>Enumerator
<tr><td>Values<td> NRT_element | NRT_average
<tr><td>Default Value<td>"NRT_element"<tr><td>Description <td>Define how to implement NRT in multielemental targets.
<br>
- NRT_element: NRT calculated per recoil atom<br>
- NRT_average: NRT calculated using material average values<br>
<tr><th colspan="2">\anchor _Simulation_intra_cascade_recombination Simulation.intra_cascade_recombination<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Enable intra-cascade recombination of Frenkel pairs.
<tr><th colspan="2">\anchor _Transport Transport<tr><td>Type <td>Option group
<tr><td>Description <td>Ion transport options
<tr><th colspan="2">\anchor _Transport_min_energy Transport.min_energy<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Lowest kinetic energy of a simulated ion.
<br>
When the energy of an ion goes below this cutoff,<br>
the ion history is terminated.<br>
<tr><th colspan="2">\anchor _Transport_flight_path_type Transport.flight_path_type<tr><td>Type <td>Enumerator
<tr><td>Values<td> AtomicSpacing | Constant | MendenhallWeller | FullMC
<tr><td>Default Value<td>"AtomicSpacing"<tr><td>Description <td>Flight path selection algorithm.
<br>
- AtomicSpacing: Constant flight path equal to material's interatomic distance<br>
- Constant: User defined constant flight path<br>
- MendenhallWeller: SRIM-like path selection algorithm<br>
- FullMC: Full Monte-Carlo flight path algorithm<br>
<tr><th colspan="2">\anchor _Transport_flight_path_const Transport.flight_path_const<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>0.1<tr><td>Description <td>Constant Flight Path in nm.
<br>
Used when flight_path_type=Constant<br>
<tr><th colspan="2">\anchor _Transport_max_rel_eloss Transport.max_rel_eloss<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1
<tr><td>Default Value<td>0.05<tr><td>Description <td>Maximum allowed relative energy loss per flight path.
<br>
Applicable only when flight_path_type=MendenhallWeller or FullMC<br>
<tr><th colspan="2">\anchor _Transport_min_recoil_energy Transport.min_recoil_energy<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Minimum recoil energy in eV.
<br>
Applicable only when flight_path_type=MendenhallWeller or FullMC<br>
<tr><th colspan="2">\anchor _Transport_max_mfp Transport.max_mfp<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+30
<tr><td>Default Value<td>1e+30<tr><td>Description <td>Maximum ion mean free path in nm.
<br>
Applicable only when flight_path_type=FullMC<br>
<tr><th colspan="2">\anchor _Transport_allow_sub_ml_scattering Transport.allow_sub_ml_scattering<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Allow sub-monolayer ion flight path.
<br>
Applicable only when flight_path_type=FullMC<br>
<tr><th colspan="2">\anchor _IonBeam IonBeam<tr><td>Type <td>Option group
<tr><td>Description <td>Ion Source
<tr><th colspan="2">\anchor _IonBeam_ion IonBeam.ion<tr><td>Type <td>Option group
<tr><td>Description <td>Projectile ion definition
<tr><th colspan="2">\anchor _IonBeam_ion_symbol IonBeam.ion.symbol<tr><td>Type <td>String
<tr><td>Default Value<td>"H"<tr><td>Description <td>Chemical element symbol.
<tr><th colspan="2">\anchor _IonBeam_ion_atomic_number IonBeam.ion.atomic_number<tr><td>Type <td>Integer
<tr><td>Range<td>1...92
<tr><td>Default Value<td>1<tr><td>Description <td>Atomic number of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_ion_atomic_mass IonBeam.ion.atomic_mass<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>1.007825<tr><td>Description <td>Atomic mass of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution IonBeam.energy_distribution<tr><td>Type <td>Option group
<tr><td>Description <td>Energy Distribution
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_type IonBeam.energy_distribution.type<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of energy distribution of the generated ions.
<br>
- Single Value: All ions have the same energy<br>
- Uniform: Ion energy distributed uniformly within center ± fwhm/2<br>
- Gaussian: Ion energy distributed according to the Gaussian(Normal) distribution around the center value with given fwhm<br>
 <br>
When sampling from a distribution, out-of-bounds values are rejected and a new sample is drawn.<br>
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_center IonBeam.energy_distribution.center<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1e+06<tr><td>Description <td>Central energy of generated ions in eV.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_fwhm IonBeam.energy_distribution.fwhm<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions energy distribution in eV.
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution IonBeam.spatial_distribution<tr><td>Type <td>Option group
<tr><td>Description <td>Spatial Distribution
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_geometry IonBeam.spatial_distribution.geometry<tr><td>Type <td>Enumerator
<tr><td>Values<td> Surface | Volume
<tr><td>Default Value<td>"Surface"<tr><td>Description <td>Geometry of the ion source.
<br>
- Surface: Ions are generated on a simulation boundary surface.<br>
- Volume: Ions are generated within the simulation volume.<br>
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_type IonBeam.spatial_distribution.type<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of spatial distribution of the generated ions.
<br>
- Single Value: All ions have the same initial position<br>
- Uniform: Ion position distributed uniformly around the center position<br>
- Gaussian: Ion position distributed according to the Gaussian(Normal) distribution around the center position with given fwhm<br>
<br>
Surface(2D) distributions are sampled on the left yz-plane bounding the simulation box.<br>
In a Uniform surface(volume) distribution, the position is sampled uniformly in a square(cube) of width fwhm around the center.<br>
In a Gaussian distribution, each component of the position vector is sampled from a Gaussian with the same fwhm around the center.<br>
When sampling from a distribution, out-of-bounds positions are rejected and a new sample is drawn.<br>
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_center IonBeam.spatial_distribution.center<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>-1e+10...1e+10
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Central initial position of generated ions, [x,y,z] in nm.
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_fwhm IonBeam.spatial_distribution.fwhm<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions position distribution in nm.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution IonBeam.angular_distribution<tr><td>Type <td>Option group
<tr><td>Description <td>Angular Distribution
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_type IonBeam.angular_distribution.type<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of angular distribution of the generated ions.
<br>
- Single Value: All ions have the same initial direction<br>
- Uniform: Ion direction distributed uniformly within a cone around the central direction<br>
- Gaussian: Ion direction distributed according to the Gaussian(Normal) distribution around the central direction<br>
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_center IonBeam.angular_distribution.center<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>-1000...1000
<tr><td>Default Value<td>[1.0,0.0,0.0]<tr><td>Description <td>Ion beam central direction vector, [nx,ny,nz], unnormalized.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_fwhm IonBeam.angular_distribution.fwhm<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.01...1000
<tr><td>Default Value<td>1.0<tr><td>Description <td>Width in srad of a cone around the central ion beam direction.
<br>
For the Uniform distribution, fwhm defines a cone around the main direction, where the direction of generated ions is sampled uniformly<br>
The Gaussian distrubution is not yet implemented<br>
<tr><th colspan="2">\anchor _Target Target<tr><td>Type <td>Option group
<tr><td>Description <td>Target
<tr><th colspan="2">\anchor _Target_size Target.size<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>0.001...1e+07
<tr><td>Default Value<td>[100.0,100.0,100.0]<tr><td>Description <td>Size in nm of the simulation volume along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _Target_origin Target.origin<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the simulation space.
<tr><th colspan="2">\anchor _Target_cell_count Target.cell_count<tr><td>Type <td>Integer 3d Vector
<tr><td>Range<td>1...1e+06
<tr><td>Default Value<td>[1,1,1]<tr><td>Description <td>Number of simulation cells along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _Target_periodic_bc Target.periodic_bc<tr><td>Type <td>Integer 3d Vector
<tr><td>Range<td>0...1
<tr><td>Default Value<td>[0,1,1]<tr><td>Description <td>Select periodic boundary conditions along the axes (0=normal, 1=periodic).
<tr><th colspan="2">\anchor _Target_materials Target.materials<tr><td>Type <td>Option group
<tr><td>Description <td>Target materials definition
<tr><th colspan="2">\anchor _Target_regions Target.regions<tr><td>Type <td>Option group
<tr><td>Description <td>Target regions definition
<tr><th colspan="2">\anchor _Output Output<tr><td>Type <td>Option group
<tr><td>Description <td>Output options
<tr><th colspan="2">\anchor _Output_title Output.title<tr><td>Type <td>String
<tr><td>Default Value<td>"Ion Simulation"<tr><td>Description <td>Short title describing the simulation.
<tr><th colspan="2">\anchor _Output_storage_interval Output.storage_interval<tr><td>Type <td>Integer
<tr><td>Range<td>100...2.14748e+09
<tr><td>Default Value<td>1000<tr><td>Description <td>Time interval (ms) to update stored data.
<tr><th colspan="2">\anchor _Output_store_exit_events Output.store_exit_events<tr><td>Type <td>Boolean
<tr><td>Default Value<td>0<tr><td>Description <td>Store a table of ion exit events.
<tr><th colspan="2">\anchor _Output_store_pka_events Output.store_pka_events<tr><td>Type <td>Boolean
<tr><td>Default Value<td>0<tr><td>Description <td>Store a table of PKA events.
<tr><th colspan="2">\anchor _Output_store_dedx Output.store_dedx<tr><td>Type <td>Boolean
<tr><td>Default Value<td>1<tr><td>Description <td>Store electronic stopping tables for each ion/material combination.
<tr><th colspan="2">\anchor _Driver Driver<tr><td>Type <td>Option group
<tr><td>Description <td>Options for the simulation driver.
<tr><th colspan="2">\anchor _Driver_max_no_ions Driver.max_no_ions<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>100<tr><td>Description <td>Maximum number of ion histories to simulate.
<tr><th colspan="2">\anchor _Driver_threads Driver.threads<tr><td>Type <td>Integer
<tr><td>Range<td>1...100
<tr><td>Default Value<td>1<tr><td>Description <td>Number of execution threads.
<tr><th colspan="2">\anchor _Driver_seed Driver.seed<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>123456789<tr><td>Description <td>Random number generator seed.
</table>
