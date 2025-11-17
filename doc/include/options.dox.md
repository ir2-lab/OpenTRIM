## JSON config string

> <br>
{<br>
&emsp;&emsp;\ref _Simulation "\"Simulation\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_simulation_type "\"simulation_type\"": "FullCascade",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_screening_type "\"screening_type\"": "ZBL",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_electronic_stopping "\"electronic_stopping\"": "SRIM13",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_electronic_straggling "\"electronic_straggling\"": "Off",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_nrt_calculation "\"nrt_calculation\"": "NRT_element",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_intra_cascade_recombination "\"intra_cascade_recombination\"": false<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Transport "\"Transport\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_min_energy "\"min_energy\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_flight_path_type "\"flight_path_type\"": "Constant",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_flight_path_const "\"flight_path_const\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_max_rel_eloss "\"max_rel_eloss\"": 0.05,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_min_recoil_energy "\"min_recoil_energy\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_min_scattering_angle "\"min_scattering_angle\"": 2.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Transport_mfp_range "\"mfp_range\"": [1.0,1e+30]<br>
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
&emsp;&emsp;&emsp;&emsp;\ref _Target_materials "\"materials\"": [<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_id "\"id\"": "Iron",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_density "\"density\"": 7.8658,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_color "\"color\"": "#55aaff",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition "\"composition\"": [<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_element "\"element\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_element_symbol "\"symbol\"": "Fe",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_element_atomic_number "\"atomic_number\"": 26,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_element_atomic_mass "\"atomic_mass\"": 55.8452<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_X "\"X\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_Ed "\"Ed\"": 40.0,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_El "\"El\"": 3.0,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_Es "\"Es\"": 10.0,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_Er "\"Er\"": 40.0,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_materials_0_composition_0_Rc "\"Rc\"": 0.946<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;]<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;],<br>
&emsp;&emsp;&emsp;&emsp;\ref _Target_regions "\"regions\"": [<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_regions_0_id "\"id\"": "R1",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_regions_0_material_id "\"material_id\"": "Iron",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_regions_0_origin "\"origin\"": [0.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _Target_regions_0_size "\"size\"": [100.0,100.0,100.0]<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;]<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Output "\"Output\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_title "\"title\"": "Ion Simulation",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_outfilename "\"outfilename\"": "out",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_storage_interval "\"storage_interval\"": 1000,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_exit_events "\"store_exit_events\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_pka_events "\"store_pka_events\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_damage_events "\"store_damage_events\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_dedx "\"store_dedx\"": true<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Run "\"Run\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Run_max_no_ions "\"max_no_ions\"": 100,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Run_threads "\"threads\"": 1,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Run_seed "\"seed\"": 123456789<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _UserTally "\"UserTally\"": [<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_id "\"id\"": "UserTally0",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_description "\"description\"": "",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_event "\"event\"": "IonStop",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_coordinate_system "\"coordinate_system\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_coordinate_system_origin "\"origin\"": [0.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_coordinate_system_zaxis "\"zaxis\"": [0.0,0.0,1.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_coordinate_system_xzvector "\"xzvector\"": [1.0,0.0,1.0]<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_x "\"x\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_y "\"y\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_z "\"z\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_r "\"r\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_rho "\"rho\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_cosTheta "\"cosTheta\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_nx "\"nx\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_ny "\"ny\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_nz "\"nz\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_E "\"E\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_Tdam "\"Tdam\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_V "\"V\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_atom_id "\"atom_id\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_recoil_id "\"recoil_id\"": []<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;]<br>
}<br>
<br>

## Detailed description



<table>
<caption>OpenTRIM JSON config - Detailed Description</caption>
<tr><th colspan="2">\anchor _Simulation /Simulation<tr><td>Type <td>Option group
<tr><td>Description <td>General simulation options
<tr><th colspan="2">\anchor _Simulation_simulation_type /Simulation/simulation_type<tr><td>Type <td>Enumerator
<tr><td>Values<td> FullCascade | IonsOnly | CascadesOnly
<tr><td>Default Value<td>"FullCascade"<tr><td>Description <td>Define the type of simulation.
<br>
- FullCascade: Simulate full damage cascades, follow recoils<br>
- IonsOnly: Simulate only ions, do not follow recoils<br>
- CascadesOnly: Simulate recoil cascades<br>
<tr><th colspan="2">\anchor _Simulation_screening_type /Simulation/screening_type<tr><td>Type <td>Enumerator
<tr><td>Values<td> None | Bohr | KrC | Moliere | ZBL | ZBL_MAGIC
<tr><td>Default Value<td>"ZBL"<tr><td>Description <td>Define the type of screening potential.
<br>
- None: Unscreened Coulomb potential<br>
- Bohr: Bohr screening<br>
- KrC: Kr-C screening<br>
- Moliere - Moliere screening<br>
- ZBL - Ziegler-Biersack-Littmark universal screening<br>
- ZBL_MAGIC - ZBL with analytic approx. formula<br>
  <br>
For Unscreened Coulomb and ZBL_MAGIK the scattering is calculated analytically.<br>
In all other cases interpolation tables are used.<br>
<tr><th colspan="2">\anchor _Simulation_electronic_stopping /Simulation/electronic_stopping<tr><td>Type <td>Enumerator
<tr><td>Values<td> Off | SRIM96 | SRIM13 | DPASS22
<tr><td>Default Value<td>"SRIM13"<tr><td>Description <td>Calculation of electronic stopping.
<br>
- Off: No electronic stopping<br>
- SRIM96: Using the SRIM parametrization, v. 1996<br>
- SRIM13: Using the SRIM parametrization, v. 2013<br>
- DPASS22: Using the DPASS parametrization, v. 21.06<br>
<tr><th colspan="2">\anchor _Simulation_electronic_straggling /Simulation/electronic_straggling<tr><td>Type <td>Enumerator
<tr><td>Values<td> Off | Bohr | Chu | Yang
<tr><td>Default Value<td>"Off"<tr><td>Description <td>Calculation of electronic straggling.
<br>
- Off: No electronic straggling.<br>
- Bohr: According to the Bohr model<br>
- Chu: According to the model of Chu et al.<br>
- Yang: According to the model of Yang et al.<br>
<tr><th colspan="2">\anchor _Simulation_nrt_calculation /Simulation/nrt_calculation<tr><td>Type <td>Enumerator
<tr><td>Values<td> NRT_element | NRT_average
<tr><td>Default Value<td>"NRT_element"<tr><td>Description <td>Define how to implement NRT in multielemental targets.
<br>
- NRT_element: NRT calculated per recoil atom<br>
- NRT_average: NRT calculated using material average values<br>
<tr><th colspan="2">\anchor _Simulation_intra_cascade_recombination /Simulation/intra_cascade_recombination<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Enable intra-cascade recombination of Frenkel pairs.
<tr><th colspan="2">\anchor _Transport /Transport<tr><td>Type <td>Option group
<tr><td>Description <td>Ion transport options
<tr><th colspan="2">\anchor _Transport_min_energy /Transport/min_energy<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Lowest kinetic energy of a simulated ion.
<br>
When the energy of an ion goes below this cutoff,<br>
the ion history is terminated.<br>
<tr><th colspan="2">\anchor _Transport_flight_path_type /Transport/flight_path_type<tr><td>Type <td>Enumerator
<tr><td>Values<td> Constant | Variable
<tr><td>Default Value<td>"Constant"<tr><td>Description <td>Flight path sampling algorithm.
<br>
- Constant: User defined constant flight path<br>
- Variable: Sampled flight path, energy dependent mean free path<br>
<tr><th colspan="2">\anchor _Transport_flight_path_const /Transport/flight_path_const<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Constant Flight Path in units of the atomic radius.
<br>
Used when flight_path_type=Constant<br>
<tr><th colspan="2">\anchor _Transport_max_rel_eloss /Transport/max_rel_eloss<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1
<tr><td>Default Value<td>0.05<tr><td>Description <td>Maximum allowed relative energy loss per flight path.
<br>
Applicable only when flight_path_type=Variable<br>
<tr><th colspan="2">\anchor _Transport_min_recoil_energy /Transport/min_recoil_energy<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Minimum recoil energy in eV.
<br>
Events with recoil energy below this value will be ignoredif the scattering angle is also below min_scattering_angle.<br>
Applicable only when flight_path_type=Variable<br>
<tr><th colspan="2">\anchor _Transport_min_scattering_angle /Transport/min_scattering_angle<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...90
<tr><td>Default Value<td>2.0<tr><td>Description <td>Minimum scattering angle in degrees.
<br>
Refers to the projectile scattering angle in the lab reference frame.<br>
Events with scattering angle lower that this value will be ignotred if the recoil energy is also below min_recoil_energy.<br>
Applicable only when flight_path_type=Variable<br>
<tr><th colspan="2">\anchor _Transport_mfp_range /Transport/mfp_range<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>2
<tr><td>Element range<td>0.001...1e+30
<tr><td>Default Value<td>[1.0,1e+30]<tr><td>Description <td>Range of ion mean free path in units of atomic radius.
<br>
Applicable only when flight_path_type=Variable<br>
<tr><th colspan="2">\anchor _IonBeam /IonBeam<tr><td>Type <td>Option group
<tr><td>Description <td>Ion Source
<tr><th colspan="2">\anchor _IonBeam_ion /IonBeam/ion<tr><td>Type <td>Option group
<tr><td>Description <td>Projectile ion definition
<tr><th colspan="2">\anchor _IonBeam_ion_symbol /IonBeam/ion/symbol<tr><td>Type <td>String
<tr><td>Default Value<td>"H"<tr><td>Description <td>Chemical element symbol.
<tr><th colspan="2">\anchor _IonBeam_ion_atomic_number /IonBeam/ion/atomic_number<tr><td>Type <td>Integer
<tr><td>Range<td>1...92
<tr><td>Default Value<td>1<tr><td>Description <td>Atomic number of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_ion_atomic_mass /IonBeam/ion/atomic_mass<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>1.007825<tr><td>Description <td>Atomic mass of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution /IonBeam/energy_distribution<tr><td>Type <td>Option group
<tr><td>Description <td>Energy Distribution
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_type /IonBeam/energy_distribution/type<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of energy distribution of the generated ions.
<br>
- Single Value: All ions have the same energy<br>
- Uniform: Ion energy distributed uniformly within center ± fwhm/2<br>
- Gaussian: Ion energy distributed according to the Gaussian(Normal) distribution around the center value with given fwhm<br>
 <br>
When sampling from a distribution, out-of-bounds values are rejected and a new sample is drawn.<br>
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_center /IonBeam/energy_distribution/center<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1e+06<tr><td>Description <td>Central energy of generated ions in eV.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_fwhm /IonBeam/energy_distribution/fwhm<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions energy distribution in eV.
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution /IonBeam/spatial_distribution<tr><td>Type <td>Option group
<tr><td>Description <td>Spatial Distribution
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_geometry /IonBeam/spatial_distribution/geometry<tr><td>Type <td>Enumerator
<tr><td>Values<td> Surface | Volume
<tr><td>Default Value<td>"Surface"<tr><td>Description <td>Geometry of the ion source.
<br>
- Surface: Ions are generated on a simulation boundary surface.<br>
- Volume: Ions are generated within the simulation volume.<br>
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_type /IonBeam/spatial_distribution/type<tr><td>Type <td>Enumerator
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
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_center /IonBeam/spatial_distribution/center<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+10...1e+10
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Central initial position of generated ions, [x,y,z] in nm.
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_fwhm /IonBeam/spatial_distribution/fwhm<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions position distribution in nm.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution /IonBeam/angular_distribution<tr><td>Type <td>Option group
<tr><td>Description <td>Angular Distribution
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_type /IonBeam/angular_distribution/type<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of angular distribution of the generated ions.
<br>
- Single Value: All ions have the same initial direction<br>
- Uniform: Ion direction distributed uniformly within a cone around the central direction<br>
- Gaussian: Ion direction distributed according to the Gaussian(Normal) distribution around the central direction<br>
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_center /IonBeam/angular_distribution/center<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1000...1000
<tr><td>Default Value<td>[1.0,0.0,0.0]<tr><td>Description <td>Ion beam central direction vector, [nx,ny,nz], unnormalized.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_fwhm /IonBeam/angular_distribution/fwhm<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.01...1000
<tr><td>Default Value<td>1.0<tr><td>Description <td>Width in srad of a cone around the central ion beam direction.
<br>
For the Uniform distribution, fwhm defines a cone around the main direction, where the direction of generated ions is sampled uniformly<br>
The Gaussian distrubution is not yet implemented<br>
<tr><th colspan="2">\anchor _Target /Target<tr><td>Type <td>Option group
<tr><td>Description <td>Target
<tr><th colspan="2">\anchor _Target_size /Target/size<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>0.001...1e+07
<tr><td>Default Value<td>[100.0,100.0,100.0]<tr><td>Description <td>Size in nm of the simulation volume along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _Target_origin /Target/origin<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the simulation space.
<tr><th colspan="2">\anchor _Target_cell_count /Target/cell_count<tr><td>Type <td>Vector of integer values
<tr><td>Size<td>3
<tr><td>Element range<td>1...1e+06
<tr><td>Default Value<td>[1,1,1]<tr><td>Description <td>Number of simulation cells along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _Target_periodic_bc /Target/periodic_bc<tr><td>Type <td>Vector of integer values
<tr><td>Size<td>3
<tr><td>Element range<td>0...1
<tr><td>Default Value<td>[0,1,1]<tr><td>Description <td>Select periodic boundary conditions along the axes (0=normal, 1=periodic).
<tr><th colspan="2">\anchor _Target_materials /Target/materials<tr><td>Type <td>Array of same type options
<tr><td>Description <td>Target materials definition
<tr><th colspan="2">\anchor _Target_materials_0_id /Target/materials/0/id<tr><td>Type <td>String
<tr><td>Default Value<td>"Iron"<tr><td>Description <td>Name of the material.
<tr><th colspan="2">\anchor _Target_materials_0_density /Target/materials/0/density<tr><td>Type <td>Floating point number
<tr><td>Range<td>1e-06...1e+06
<tr><td>Default Value<td>7.8658<tr><td>Description <td>Mass density [g/cm3]
<tr><th colspan="2">\anchor _Target_materials_0_color /Target/materials/0/color<tr><td>Type <td>String
<tr><td>Default Value<td>"#55aaff"<tr><td>Description <td>Html color code.
<tr><th colspan="2">\anchor _Target_materials_0_composition /Target/materials/0/composition<tr><td>Type <td>Array of same type options
<tr><td>Description <td>Material composition
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element /Target/materials/0/composition/0/element<tr><td>Type <td>Option group
<tr><td>Description <td>Element definition
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element_symbol /Target/materials/0/composition/0/element/symbol<tr><td>Type <td>String
<tr><td>Default Value<td>"Fe"<tr><td>Description <td>Chemical element symbol.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element_atomic_number /Target/materials/0/composition/0/element/atomic_number<tr><td>Type <td>Integer
<tr><td>Range<td>1...92
<tr><td>Default Value<td>26<tr><td>Description <td>Element atomic number.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element_atomic_mass /Target/materials/0/composition/0/element/atomic_mass<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>55.8452<tr><td>Description <td>Element atomic mass.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_X /Target/materials/0/composition/0/X<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>1.0<tr><td>Description <td>Relative atomic concentration (un-normalized).
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_Ed /Target/materials/0/composition/0/Ed<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>40.0<tr><td>Description <td>Displacement energy [eV].
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_El /Target/materials/0/composition/0/El<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>3.0<tr><td>Description <td>Lattice binding energy [eV].
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_Es /Target/materials/0/composition/0/Es<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>10.0<tr><td>Description <td>Surface binding energy [eV].
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_Er /Target/materials/0/composition/0/Er<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>40.0<tr><td>Description <td>Replacement energy [eV].
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_Rc /Target/materials/0/composition/0/Rc<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>0.946<tr><td>Description <td>Recombination radius [nm].
<tr><th colspan="2">\anchor _Target_regions /Target/regions<tr><td>Type <td>Array of same type options
<tr><td>Description <td>Target regions definition
<tr><th colspan="2">\anchor _Target_regions_0_id /Target/regions/0/id<tr><td>Type <td>String
<tr><td>Default Value<td>"R1"<tr><td>Description <td>Name of the region.
<tr><th colspan="2">\anchor _Target_regions_0_material_id /Target/regions/0/material_id<tr><td>Type <td>String
<tr><td>Default Value<td>"Iron"<tr><td>Description <td>Id of the material that fills the region.
<tr><th colspan="2">\anchor _Target_regions_0_origin /Target/regions/0/origin<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the region.
<tr><th colspan="2">\anchor _Target_regions_0_size /Target/regions/0/size<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>0.001...1e+07
<tr><td>Default Value<td>[100.0,100.0,100.0]<tr><td>Description <td>Size in nm of the simulation volume along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _Output /Output<tr><td>Type <td>Option group
<tr><td>Description <td>Output options
<tr><th colspan="2">\anchor _Output_title /Output/title<tr><td>Type <td>String
<tr><td>Default Value<td>"Ion Simulation"<tr><td>Description <td>Short title describing the simulation.
<tr><th colspan="2">\anchor _Output_outfilename /Output/outfilename<tr><td>Type <td>String
<tr><td>Default Value<td>"out"<tr><td>Description <td>Name of the output file without the extention.
<br>
The extention '.h5' will be added.<br>
The name can contain the relative or absolute path to the file.<br>
<tr><th colspan="2">\anchor _Output_storage_interval /Output/storage_interval<tr><td>Type <td>Integer
<tr><td>Range<td>100...2.14748e+09
<tr><td>Default Value<td>1000<tr><td>Description <td>Time interval (ms) to update stored data.
<tr><th colspan="2">\anchor _Output_store_exit_events /Output/store_exit_events<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Store a table of ion exit events.
<tr><th colspan="2">\anchor _Output_store_pka_events /Output/store_pka_events<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Store a table of PKA events.
<tr><th colspan="2">\anchor _Output_store_damage_events /Output/store_damage_events<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Store a table of damage events.
<tr><th colspan="2">\anchor _Output_store_dedx /Output/store_dedx<tr><td>Type <td>Boolean
<tr><td>Default Value<td>true<tr><td>Description <td>Store electronic stopping tables for each ion/material combination.
<tr><th colspan="2">\anchor _Run /Run<tr><td>Type <td>Option group
<tr><td>Description <td>Options for running the simulation.
<tr><th colspan="2">\anchor _Run_max_no_ions /Run/max_no_ions<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>100<tr><td>Description <td>Maximum number of ion histories to simulate.
<tr><th colspan="2">\anchor _Run_threads /Run/threads<tr><td>Type <td>Integer
<tr><td>Range<td>1...100
<tr><td>Default Value<td>1<tr><td>Description <td>Number of execution threads.
<tr><th colspan="2">\anchor _Run_seed /Run/seed<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>123456789<tr><td>Description <td>Random number generator seed.
<tr><th colspan="2">\anchor _UserTally /UserTally<tr><td>Type <td>Array of same type options
<tr><td>Description <td>Array of user-defined tallies
<tr><th colspan="2">\anchor _UserTally_0_id /UserTally/0/id<tr><td>Type <td>String
<tr><td>Default Value<td>"UserTally0"<tr><td>Description <td>User specified tally id (name).
<br>
For identifying the specific UserTally<br>
in the output file etc.<br>
<tr><th colspan="2">\anchor _UserTally_0_description /UserTally/0/description<tr><td>Type <td>String
<tr><td>Default Value<td>""<tr><td>Description <td>Short description of the tally.
<br>
One can give here a short explanation of<br>
the purpose of setting up this tally,<br>
what kind of information is expected, etc.<br>
<tr><th colspan="2">\anchor _UserTally_0_event /UserTally/0/event<tr><td>Type <td>Enumerator
<tr><td>Values<td> IonExit | IonStop | Vacancy | Replacement | CascadeComplete | BoundaryCrossing
<tr><td>Default Value<td>"IonStop"<tr><td>Description <td>Simulation event that will trigger a tally score.
<br>
The supported events are:<br>
- IonExit: the ion exits the simulation volume<br>
- IonStop: the ion stops inside the simulation volume<br>
- Vacancy: a lattice vacancy is created<br>
- Replacement: a replacement event occurs<br>
- CascadeComplete: a PKA cascade is completed<br>
- BoundaryCrossing: an ion crosses an internal cell boundary<br>
<tr><th colspan="2">\anchor _UserTally_0_coordinate_system /UserTally/0/coordinate_system<tr><td>Type <td>Option group
<tr><td>Description <td>UserTally coordinate system
<tr><th colspan="2">\anchor _UserTally_0_coordinate_system_origin /UserTally/0/coordinate_system/origin<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the UserTally coordinates.
<tr><th colspan="2">\anchor _UserTally_0_coordinate_system_zaxis /UserTally/0/coordinate_system/zaxis<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,1.0]<tr><td>Description <td>A vector parallel to the z-axis of the UserTally coordinate system.
<tr><th colspan="2">\anchor _UserTally_0_coordinate_system_xzvector /UserTally/0/coordinate_system/xzvector<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[1.0,0.0,1.0]<tr><td>Description <td>A vector on the xz-plane of the UserTally coordinate system.
<tr><th colspan="2">\anchor _UserTally_0_x /UserTally/0/x<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion position x-coordinate.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_y /UserTally/0/y<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion position y-coordinate.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_z /UserTally/0/z<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion position z-coordinate.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_r /UserTally/0/r<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's r=sqrt(x^2+y^2+z^2).
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_rho /UserTally/0/rho<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's ρ=sqrt(x^2+y^2).
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_cosTheta /UserTally/0/cosTheta<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's position cosθ = z/r.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_nx /UserTally/0/nx<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's x-axis direction cosine.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_ny /UserTally/0/ny<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's y-axis direction cosine.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_nz /UserTally/0/nz<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's z-axis direction cosine.
<br>
Coordinates refer to the UserTally coordinate system.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_E /UserTally/0/E<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's kinetic energy.
<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_Tdam /UserTally/0/Tdam<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's damage energy.
<br>
Tdam refers to the energy of the PKA dissipated to atomic displacements during the whole cascade<br>
This can be used only with events of type CascadeComplete<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_V /UserTally/0/V<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the number of generated vacancies.
<br>
It refers to the total number of vacancies generated in a PKA cascade<br>
This can be used only with events of type CascadeComplete<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_atom_id /UserTally/0/atom_id<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's atomic species id.
<br>
Each atomic species in the simulation is assigned an id number<br>
Beam ions always have an id of 0.<br>
Target atoms recoils have id>=1.<br>
Bin edges must be monotonously increasing.<br>
<tr><th colspan="2">\anchor _UserTally_0_recoil_id /UserTally/0/recoil_id<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's recoil generation id.
<br>
Beam ions always have a recoil generation id of 0.<br>
PKAs have id=1. Higher order recoils have higher recoil ids<br>
Bin edges must be monotonously increasing.<br>
</table>
