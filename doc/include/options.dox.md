## JSON config string

> <br>
{<br>
&emsp;&emsp;\ref _Simulation "\"Simulation\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_simulation_type "\"simulation_type\"": "FullCascade",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_screening_type "\"screening_type\"": "ZBL",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_electronic_stopping "\"electronic_stopping\"": "SRIM13",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_electronic_straggling "\"electronic_straggling\"": "Off",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_nrt_calculation "\"nrt_calculation\"": "NRT_element",<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_intra_cascade_recombination "\"intra_cascade_recombination\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_time_ordered_cascades "\"time_ordered_cascades\"": true,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_correlated_recombination "\"correlated_recombination\"": true,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_move_recoil "\"move_recoil\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Simulation_recoil_sub_ed "\"recoil_sub_ed\"": false<br>
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
&emsp;&emsp;&emsp;&emsp;\ref _Output_storage_interval "\"storage_interval\"": 60000,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_exit_events "\"store_exit_events\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_pka_events "\"store_pka_events\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_damage_events "\"store_damage_events\"": false,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Output_store_dedx "\"store_dedx\"": true<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _Run "\"Run\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _Run_max_no_ions "\"max_no_ions\"": 100,<br>
&emsp;&emsp;&emsp;&emsp;\ref _Run_max_cpu_time "\"max_cpu_time\"": 0,<br>
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
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins "\"bins\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_x "\"x\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_y "\"y\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_z "\"z\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_r "\"r\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_rho "\"rho\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_cosTheta "\"cosTheta\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_nx "\"nx\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_ny "\"ny\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_nz "\"nz\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_E "\"E\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_Tdam "\"Tdam\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_V "\"V\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_atom_id "\"atom_id\"": [],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _UserTally_0_bins_recoil_id "\"recoil_id\"": []<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;]<br>
}<br>
<br>

## Detailed description



<table>
<caption>OpenTRIM JSON config - Detailed Description</caption>
<tr><th colspan="2">\anchor _Simulation /Simulation<tr><td>Label <td>General simulation options
<tr><td>Type <td>Option group
<tr><td>Description <td>General options controlling how the simulation is carried out.
<tr><th colspan="2">\anchor _Simulation_simulation_type /Simulation/simulation_type<tr><td>Label <td>Simulation Type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> FullCascade | IonsOnly | CascadesOnly
<tr><td>Default Value<td>"FullCascade"<tr><td>Description <td>Define the type of simulation.
<h4>Options</h4><ul><li><strong>FullCascade</strong> [Full Cascade (Ions &amp; Cascades)] - Simulate full damage cascades, follow recoils</li><li><strong>IonsOnly</strong> [Ions Only] - Simulate only ions, do not follow recoils</li><li><strong>CascadesOnly</strong> [Cascades Only] - Simulate recoil cascades</li></ul><tr><th colspan="2">\anchor _Simulation_screening_type /Simulation/screening_type<tr><td>Label <td>Screening Type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> None | Bohr | KrC | Moliere | ZBL | ZBL_MAGIC
<tr><td>Default Value<td>"ZBL"<tr><td>Description <td>Define the type of screening potential.
<h4>Options</h4><ul><li><strong>None</strong> [None (Unscreened)] - Unscreened Coulomb potential</li><li><strong>Bohr</strong> - Bohr screening</li><li><strong>KrC</strong> - Kr-C screening</li><li><strong>Moliere</strong> - Moliere screening</li><li><strong>ZBL</strong> - Ziegler-Biersack-Littmark universal screening</li><li><strong>ZBL_MAGIC</strong> [ZBL_MAGIC (Analytic approx.)] - ZBL with analytic approx. formula</li></ul><h4>Notes</h4><ul><li>For Unscreened Coulomb and ZBL_MAGIC the scattering is calculated analytically.</li>
<li>In all other cases interpolation tables are used.</li>
</ul><tr><th colspan="2">\anchor _Simulation_electronic_stopping /Simulation/electronic_stopping<tr><td>Label <td>Electronic Stopping
<tr><td>Type <td>Enumerator
<tr><td>Values<td> Off | SRIM96 | SRIM13 | DPASS
<tr><td>Default Value<td>"SRIM13"<tr><td>Description <td>Calculation of electronic stopping.
<h4>Options</h4><ul><li><strong>Off</strong> - No electronic stopping</li><li><strong>SRIM96</strong> - Using the SRIM parametrization, v. 1996</li><li><strong>SRIM13</strong> - Using the SRIM parametrization, v. 2013</li><li><strong>DPASS</strong> - Using the DPASS parametrization, v. 21.06</li></ul><tr><th colspan="2">\anchor _Simulation_electronic_straggling /Simulation/electronic_straggling<tr><td>Label <td>Electronic Straggling
<tr><td>Type <td>Enumerator
<tr><td>Values<td> Off | Bohr | Chu | Yang
<tr><td>Default Value<td>"Off"<tr><td>Description <td>Calculation of electronic straggling.
<h4>Options</h4><ul><li><strong>Off</strong> - No electronic straggling.</li><li><strong>Bohr</strong> - According to the Bohr model</li><li><strong>Chu</strong> - According to the model of Chu et al.</li><li><strong>Yang</strong> - According to the model of Yang et al.</li></ul><tr><th colspan="2">\anchor _Simulation_nrt_calculation /Simulation/nrt_calculation<tr><td>Label <td>NRT calculation
<tr><td>Type <td>Enumerator
<tr><td>Values<td> NRT_element | NRT_average
<tr><td>Default Value<td>"NRT_element"<tr><td>Description <td>Define how to implement NRT in multielemental targets.
<h4>Options</h4><ul><li><strong>NRT_element</strong> [NRT per element] - NRT calculated per recoil atom</li><li><strong>NRT_average</strong> [NRT per material average] - NRT calculated using material average values</li></ul><h4>Notes</h4><ul><li>Material average Ed is calculated as 1/Ed = Σi{Xi / Edi} according to Ghoniem &amp; Chou JNM1988</li>
</ul><tr><th colspan="2">\anchor _Simulation_intra_cascade_recombination /Simulation/intra_cascade_recombination<tr><td>Label <td>Intra-cascade recombination
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Enable intra-cascade recombination of Frenkel pairs.
<tr><th colspan="2">\anchor _Simulation_time_ordered_cascades /Simulation/time_ordered_cascades<tr><td>Label <td>Time ordered recombinations in cascades [Experimental]
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>true<tr><td>Description <td>Time ordered recombinations in cascades [Experimental]
<tr><th colspan="2">\anchor _Simulation_correlated_recombination /Simulation/correlated_recombination<tr><td>Label <td>Allow same Frenkel pair recombination [Experimental]
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>true<tr><td>Description <td>Allow same Frenkel pair recombination [Experimental]
<tr><th colspan="2">\anchor _Simulation_move_recoil /Simulation/move_recoil<tr><td>Label <td>Move recoil atom to Rc [Experimental]
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Move recoil atom to Rc [Experimental]
<tr><th colspan="2">\anchor _Simulation_recoil_sub_ed /Simulation/recoil_sub_ed<tr><td>Label <td>Subtract Ed from recoil energy [Experimental]
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Subtract Ed from recoil energy [Experimental]
<tr><th colspan="2">\anchor _Transport /Transport<tr><td>Label <td>Ion transport options
<tr><td>Type <td>Option group
<tr><td>Description <td>Options controlling the transport of simulated ions, such as flight path sampling and energy cutoffs.
<tr><th colspan="2">\anchor _Transport_min_energy /Transport/min_energy<tr><td>Label <td>Energy cutoff (eV)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Lowest kinetic energy of a simulated ion.
<h4>Notes</h4><ul><li>When the energy of an ion goes below this cutoff, the ion history is terminated.</li>
</ul><tr><th colspan="2">\anchor _Transport_flight_path_type /Transport/flight_path_type<tr><td>Label <td>Flight Path Type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> Constant | Variable
<tr><td>Default Value<td>"Constant"<tr><td>Description <td>Flight path sampling algorithm.
<h4>Options</h4><ul><li><strong>Constant</strong> - User-defined constant flight path</li><li><strong>Variable</strong> - Sampled flight path, energy-dependent mean free path</li></ul><tr><th colspan="2">\anchor _Transport_flight_path_const /Transport/flight_path_const<tr><td>Label <td>Const. Flight Path (Rat)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Constant Flight Path in units of the atomic radius, Rat.
<h4>Notes</h4><ul><li>Used when the flight path sampling algorithm is set to Constant, flight_path_type=Constant.</li>
</ul><tr><th colspan="2">\anchor _Transport_max_rel_eloss /Transport/max_rel_eloss<tr><td>Label <td>Max (ΔE/E)e
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1
<tr><td>Default Value<td>0.05<tr><td>Description <td>Maximum allowed relative electronic energy loss per flight path.
<h4>Notes</h4><ul><li>If this value is exceeded in a given flight path Δs, then Δs is truncated so that the ion suffers the maximum allowed relative energy loss and no collision takes place.</li>
<li>Applicable only when flight_path_type=Variable.</li>
</ul><tr><th colspan="2">\anchor _Transport_min_recoil_energy /Transport/min_recoil_energy<tr><td>Label <td>Min recoil E (eV)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Minimum recoil energy in eV.
<h4>Notes</h4><ul><li>Recoils with energy below this value will be ignored if the scattering angle is also below min_scattering_angle.</li>
<li>Applicable only when flight_path_type=Variable.</li>
</ul><tr><th colspan="2">\anchor _Transport_min_scattering_angle /Transport/min_scattering_angle<tr><td>Label <td>Min θ (º)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...90
<tr><td>Default Value<td>2.0<tr><td>Description <td>Minimum scattering angle θ in degrees.
<h4>Notes</h4><ul><li>Refers to the projectile scattering angle in the lab reference frame.</li>
<li>Events with scattering angle lower than this value will be ignored if the recoil energy is also below min_recoil_energy.</li>
<li>Applicable only when flight_path_type=Variable.</li>
</ul><tr><th colspan="2">\anchor _Transport_mfp_range /Transport/mfp_range<tr><td>Label <td>Ion mean free path range (Rat)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>2
<tr><td>Element range<td>0.001...1e+30
<tr><td>Default Value<td>[1.0,1e+30]<tr><td>Description <td>Defines the lower and upper limit of the ion mean free path in units of the atomic radius, Rat.
<h4>Notes</h4><ul><li>The lower limit is typically set equal to 1 Rat. This is equivalent to a cutoff radius Rc for the potential, V(R&gt;Rc)=0, where Rc~Rat.</li>
<li>Setting the lower limit below 1 Rat increases Rc and allows scattering by distant atoms.</li>
<li>The upper limit can be useful for the simulation of light atoms in thin targets, where the mfp can become much larger than the target thickness.</li>
<li>Applicable only when flight_path_type=Variable.</li>
</ul><tr><th colspan="2">\anchor _IonBeam /IonBeam<tr><td>Label <td>Ion Source
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of the ion beam source: projectile species, energy, spatial and angular distributions.
<tr><th colspan="2">\anchor _IonBeam_ion /IonBeam/ion<tr><td>Label <td>Projectile ion definition
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of the projectile ion species and mass.
<tr><th colspan="2">\anchor _IonBeam_ion_symbol /IonBeam/ion/symbol<tr><td>Label <td>Symbol
<tr><td>Type <td>String
<tr><td>Default Value<td>"H"<tr><td>Description <td>Chemical element symbol.
<tr><th colspan="2">\anchor _IonBeam_ion_atomic_number /IonBeam/ion/atomic_number<tr><td>Label <td>Atomic number
<tr><td>Type <td>Integer
<tr><td>Range<td>1...92
<tr><td>Default Value<td>1<tr><td>Description <td>Atomic number of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_ion_atomic_mass /IonBeam/ion/atomic_mass<tr><td>Label <td>Atomic mass
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>1.007825<tr><td>Description <td>Atomic mass of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution /IonBeam/energy_distribution<tr><td>Label <td>Energy Distribution
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of the kinetic energy distribution of the generated ions.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_type /IonBeam/energy_distribution/type<tr><td>Label <td>Distribution type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of energy distribution of the generated ions.
<h4>Options</h4><ul><li><strong>SingleValue</strong> [Single Value] - All ions have the same energy</li><li><strong>Uniform</strong> - Ion energy distributed uniformly within center ± fwhm/2</li><li><strong>Gaussian</strong> - Ion energy distributed according to the Gaussian (Normal) distribution around the center value with given fwhm</li></ul><h4>Notes</h4><ul><li>When sampling from a distribution, out-of-bounds values are rejected and a new sample is drawn.</li>
</ul><tr><th colspan="2">\anchor _IonBeam_energy_distribution_center /IonBeam/energy_distribution/center<tr><td>Label <td>Central energy (eV)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1e+06<tr><td>Description <td>Center (mean) of the generated ion energy distribution in eV.
<tr><th colspan="2">\anchor _IonBeam_energy_distribution_fwhm /IonBeam/energy_distribution/fwhm<tr><td>Label <td>FWHM (eV)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions energy distribution in eV.
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution /IonBeam/spatial_distribution<tr><td>Label <td>Spatial Distribution
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of the generated ions spatial distribution.
<tr><th colspan="2">\anchor _IonBeam_spatial_distribution_geometry /IonBeam/spatial_distribution/geometry<tr><td>Label <td>Source geometry
<tr><td>Type <td>Enumerator
<tr><td>Values<td> Surface | Volume
<tr><td>Default Value<td>"Surface"<tr><td>Description <td>Geometry of the ion source.
<h4>Options</h4><ul><li><strong>Surface</strong> - Ions are generated on the left yz-plane bounding the simulation volume.</li><li><strong>Volume</strong> - Ions are generated within the simulation volume.</li></ul><tr><th colspan="2">\anchor _IonBeam_spatial_distribution_type /IonBeam/spatial_distribution/type<tr><td>Label <td>Distribution type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of spatial distribution of the generated ions.
<h4>Options</h4><ul><li><strong>SingleValue</strong> [Single Value] - All ions have the same initial position</li><li><strong>Uniform</strong> - Ion position distributed uniformly around the center position</li><li><strong>Gaussian</strong> - Ion position distributed according to the Gaussian (Normal) distribution around the center position with given fwhm</li></ul><h4>Notes</h4><ul><li>Surface (2D) distributions are sampled on the left yz-plane bounding the simulation box.</li>
<li>In a Uniform surface (volume) distribution, the position is sampled uniformly in a square (cube) of width fwhm around the center.</li>
<li>In a Gaussian distribution, each component of the position vector is sampled from a Gaussian with the same fwhm around the center.</li>
<li>When sampling from a distribution, out-of-bounds positions are rejected and a new sample is drawn.</li>
</ul><tr><th colspan="2">\anchor _IonBeam_spatial_distribution_center /IonBeam/spatial_distribution/center<tr><td>Label <td>Center position (nm)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Center (mean) of the generated ions position distribution, [x,y,z] in nm.
<h4>Notes</h4><ul><li>Must be either within the simulation box or at the lowest yz-plane boundary.</li>
</ul><tr><th colspan="2">\anchor _IonBeam_spatial_distribution_fwhm /IonBeam/spatial_distribution/fwhm<tr><td>Label <td>FWHM (nm)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>1e-06...1e+12
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions position distribution in nm.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution /IonBeam/angular_distribution<tr><td>Label <td>Angular Distribution
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of the generated ions angular distribution.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_type /IonBeam/angular_distribution/type<tr><td>Label <td>Distribution type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of angular distribution of the generated ions.
<h4>Options</h4><ul><li><strong>SingleValue</strong> [Single Value] - All ions have the same initial direction</li><li><strong>Uniform</strong> - Ion direction distributed uniformly within a cone around the central direction</li><li><strong>Gaussian</strong> - Ion direction distributed according to a 2D isotropic Gaussian around the central direction</li></ul><h4>Notes</h4><ul><li>For the Gaussian distribution the transverse direction components tx,ty ~ N(0,sigma). Polar angle θ follows the Rayleigh distribution, azimuthal angle φ is automatically uniform.</li>
</ul><tr><th colspan="2">\anchor _IonBeam_angular_distribution_center /IonBeam/angular_distribution/center<tr><td>Label <td>Center direction
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1000...1000
<tr><td>Default Value<td>[1.0,0.0,0.0]<tr><td>Description <td>Ion beam central direction vector, [nx,ny,nz], unnormalized.
<tr><th colspan="2">\anchor _IonBeam_angular_distribution_fwhm /IonBeam/angular_distribution/fwhm<tr><td>Label <td>FWHM (srad)
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.01...1000
<tr><td>Default Value<td>1.0<tr><td>Description <td>Width in srad of a cone around the central ion beam direction.
<h4>Notes</h4><ul><li>For the Uniform distribution, fwhm defines a cone around the main direction, where the direction of generated ions is sampled uniformly</li>
<li>For the Gaussian distribution, fwhm defines the solid-angle width around the main direction, sampled via an isotropic 2D Gaussian in the transverse components</li>
</ul><tr><th colspan="2">\anchor _Target /Target<tr><td>Label <td>Target
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of the target geometry, materials and regions.
<tr><th colspan="2">\anchor _Target_size /Target/size<tr><td>Label <td>Size [Lx, Ly, Lz] (nm)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>0.001...1e+12
<tr><td>Default Value<td>[100.0,100.0,100.0]<tr><td>Description <td>Size in nm of the simulation volume along the x-, y- and z-axes.
<tr><th colspan="2">\anchor _Target_origin /Target/origin<tr><td>Label <td>Origin [x₀,y₀,z₀] (nm)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the simulation space.
<tr><th colspan="2">\anchor _Target_cell_count /Target/cell_count<tr><td>Label <td>Cell count [Nx, Ny, Nz]
<tr><td>Type <td>Vector of integer values
<tr><td>Size<td>3
<tr><td>Element range<td>1...1e+06
<tr><td>Default Value<td>[1,1,1]<tr><td>Description <td>Number of simulation cells along the x-, y- and z-axes.
<tr><th colspan="2">\anchor _Target_periodic_bc /Target/periodic_bc<tr><td>Label <td>Periodic boundary [x, y, z]
<tr><td>Type <td>Vector of integer values
<tr><td>Size<td>3
<tr><td>Element range<td>0...1
<tr><td>Default Value<td>[0,1,1]<tr><td>Description <td>Select periodic boundary conditions along the axes (0=normal, 1=periodic).
<tr><th colspan="2">\anchor _Target_materials /Target/materials<tr><td>Label <td>Target materials definition
<tr><td>Type <td>Array of same type options
<tr><td>Description <td>List of materials available to fill the target regions.
<tr><th colspan="2">\anchor _Target_materials_0_id /Target/materials/0/id<tr><td>Label <td>Material id
<tr><td>Type <td>String
<tr><td>Default Value<td>"Iron"<tr><td>Description <td>Name of the material.
<tr><th colspan="2">\anchor _Target_materials_0_density /Target/materials/0/density<tr><td>Label <td>Mass density [g/cm3]
<tr><td>Type <td>Floating point number
<tr><td>Range<td>1e-06...1e+06
<tr><td>Default Value<td>7.8658<tr><td>Description <td>Mass density [g/cm3].
<tr><th colspan="2">\anchor _Target_materials_0_color /Target/materials/0/color<tr><td>Label <td>Material color
<tr><td>Type <td>String
<tr><td>Default Value<td>"#55aaff"<tr><td>Description <td>HTML color code used for materials display.
<tr><th colspan="2">\anchor _Target_materials_0_composition /Target/materials/0/composition<tr><td>Label <td>Material composition
<tr><td>Type <td>Array of same type options
<tr><td>Description <td>List of elements making up the material's composition.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element /Target/materials/0/composition/0/element<tr><td>Label <td>Element definition
<tr><td>Type <td>Option group
<tr><td>Description <td>Definition of a chemical element.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element_symbol /Target/materials/0/composition/0/element/symbol<tr><td>Label <td>Symbol
<tr><td>Type <td>String
<tr><td>Default Value<td>"Fe"<tr><td>Description <td>Chemical element symbol.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element_atomic_number /Target/materials/0/composition/0/element/atomic_number<tr><td>Label <td>Atomic number
<tr><td>Type <td>Integer
<tr><td>Range<td>1...92
<tr><td>Default Value<td>26<tr><td>Description <td>Element atomic number.
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_element_atomic_mass /Target/materials/0/composition/0/element/atomic_mass<tr><td>Label <td>Atomic mass
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>55.8452<tr><td>Description <td>Element atomic mass (amu).
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_X /Target/materials/0/composition/0/X<tr><td>Label <td>Atomic concentration
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>1.0<tr><td>Description <td>Relative atomic concentration.
<h4>Notes</h4><ul><li>Values can be given in arbitrary units. They will be normalized internally so that Σi(Xi) = 1</li>
</ul><tr><th colspan="2">\anchor _Target_materials_0_composition_0_Ed /Target/materials/0/composition/0/Ed<tr><td>Label <td>Displacement energy [eV]
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>40.0<tr><td>Description <td>Displacement energy [eV].
<h4>Notes</h4><ul><li>Energy required to displace an atom from its atomic position.</li>
</ul><tr><th colspan="2">\anchor _Target_materials_0_composition_0_El /Target/materials/0/composition/0/El<tr><td>Label <td>Lattice binding energy [eV]
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>3.0<tr><td>Description <td>Lattice binding energy [eV].
<h4>Notes</h4><ul><li>Equivalent to the Frenkel pair formation energy.</li>
</ul><tr><th colspan="2">\anchor _Target_materials_0_composition_0_Es /Target/materials/0/composition/0/Es<tr><td>Label <td>Surface binding energy [eV]
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>10.0<tr><td>Description <td>Surface binding energy [eV].
<tr><th colspan="2">\anchor _Target_materials_0_composition_0_Er /Target/materials/0/composition/0/Er<tr><td>Label <td>Replacement energy [eV]
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>40.0<tr><td>Description <td>Replacement energy [eV].
<h4>Notes</h4><ul><li>An ion with kinetic energy E &lt; Er can be captured in a vacant lattice site.</li>
</ul><tr><th colspan="2">\anchor _Target_materials_0_composition_0_Rc /Target/materials/0/composition/0/Rc<tr><td>Label <td>Recombination radius [nm]
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>0.946<tr><td>Description <td>Recombination radius [nm].
<h4>Notes</h4><ul><li>A Frenkel pair with an I-V distance R &lt; Rc recombines spontaneously.</li>
</ul><tr><th colspan="2">\anchor _Target_regions /Target/regions<tr><td>Label <td>Target regions definition
<tr><td>Type <td>Array of same type options
<tr><td>Description <td>List of regions making up the target geometry.
<tr><th colspan="2">\anchor _Target_regions_0_id /Target/regions/0/id<tr><td>Label <td>Region id
<tr><td>Type <td>String
<tr><td>Default Value<td>"R1"<tr><td>Description <td>Name of the region.
<tr><th colspan="2">\anchor _Target_regions_0_material_id /Target/regions/0/material_id<tr><td>Label <td>Material id
<tr><td>Type <td>String
<tr><td>Default Value<td>"Iron"<tr><td>Description <td>Id of the material that fills the region.
<tr><th colspan="2">\anchor _Target_regions_0_origin /Target/regions/0/origin<tr><td>Label <td>Origin [x₀,y₀,z₀] (nm)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the region.
<tr><th colspan="2">\anchor _Target_regions_0_size /Target/regions/0/size<tr><td>Label <td>Size [Lx, Ly, Lz] (nm)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>0.001...1e+12
<tr><td>Default Value<td>[100.0,100.0,100.0]<tr><td>Description <td>Size in nm of the region along the x-, y- and z-axes.
<tr><th colspan="2">\anchor _Output /Output<tr><td>Label <td>Output options
<tr><td>Type <td>Option group
<tr><td>Description <td>Options controlling the simulation title, what data is saved and how the output file is named.
<tr><th colspan="2">\anchor _Output_title /Output/title<tr><td>Label <td>Title
<tr><td>Type <td>String
<tr><td>Default Value<td>"Ion Simulation"<tr><td>Description <td>Short title describing the simulation.
<tr><th colspan="2">\anchor _Output_outfilename /Output/outfilename<tr><td>Label <td>Output file name
<tr><td>Type <td>String
<tr><td>Default Value<td>"out"<tr><td>Description <td>Name of the output file without the extension.
<h4>Notes</h4><ul><li>The extension '.h5' will be added.</li>
<li>The name can contain the relative or absolute path to the file.</li>
</ul><tr><th colspan="2">\anchor _Output_storage_interval /Output/storage_interval<tr><td>Label <td>Update interval (ms)
<tr><td>Type <td>Integer
<tr><td>Range<td>100...2.14748e+09
<tr><td>Default Value<td>60000<tr><td>Description <td>Time interval (ms) to update stored data. [Not Implemented]
<tr><th colspan="2">\anchor _Output_store_exit_events /Output/store_exit_events<tr><td>Label <td>Store escaped ions
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Store a table of ion exit events.
<tr><th colspan="2">\anchor _Output_store_pka_events /Output/store_pka_events<tr><td>Label <td>Store PKAs
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Store a table of PKA events.
<tr><th colspan="2">\anchor _Output_store_damage_events /Output/store_damage_events<tr><td>Label <td>Store defects
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Store a table of generated vacancies and interstitials.
<tr><th colspan="2">\anchor _Output_store_dedx /Output/store_dedx<tr><td>Label <td>Store dE/dx
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>true<tr><td>Description <td>Store electronic stopping tables for each ion/material combination.
<tr><th colspan="2">\anchor _Run /Run<tr><td>Label <td>Options for running the simulation
<tr><td>Type <td>Option group
<tr><td>Description <td>Options controlling execution of the simulation run, such as ion number, threads and random seed.
<tr><th colspan="2">\anchor _Run_max_no_ions /Run/max_no_ions<tr><td>Label <td>Max # of ions to simulate
<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>100<tr><td>Description <td>Maximum number of ion histories to simulate.
<h4>Notes</h4><ul><li>The simulation will stop when the maximum number of histories has been reached.</li>
</ul><tr><th colspan="2">\anchor _Run_max_cpu_time /Run/max_cpu_time<tr><td>Label <td>Max CPU time in s
<tr><td>Type <td>Integer
<tr><td>Range<td>0...2.14748e+09
<tr><td>Default Value<td>0<tr><td>Description <td>Maximum CPU time for the simulation in s.
<h4>Notes</h4><ul><li>The simulation will stop when the maximum cpu time has been reached.</li>
<li>A value of 0 disables the cpu time limit.</li>
</ul><tr><th colspan="2">\anchor _Run_threads /Run/threads<tr><td>Label <td>Number of threads
<tr><td>Type <td>Integer
<tr><td>Range<td>0...100
<tr><td>Default Value<td>1<tr><td>Description <td>Number of execution threads.
<h4>Notes</h4><ul><li>0 means that the number of threads is selected automatically.</li>
</ul><tr><th colspan="2">\anchor _Run_seed /Run/seed<tr><td>Label <td>Random number seed
<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>123456789<tr><td>Description <td>Random number generator seed.
<tr><th colspan="2">\anchor _UserTally /UserTally<tr><td>Label <td>User tallies
<tr><td>Type <td>Array of same type options
<tr><td>Description <td>List of user-defined tallies for scoring simulation events.
<tr><th colspan="2">\anchor _UserTally_0_id /UserTally/0/id<tr><td>Label <td>User-specified tally id
<tr><td>Type <td>String
<tr><td>Default Value<td>"UserTally0"<tr><td>Description <td>User-specified tally id (name).
<h4>Notes</h4><ul><li>For identifying the specific UserTally in the output file.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_description /UserTally/0/description<tr><td>Label <td>User-specified tally description
<tr><td>Type <td>String
<tr><td>Default Value<td>""<tr><td>Description <td>Short description of the tally.
<h4>Notes</h4><ul><li>One can give here a short explanation of the tally, its purpose, expected information, etc.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_event /UserTally/0/event<tr><td>Label <td>Type of simulation event to tally
<tr><td>Type <td>Enumerator
<tr><td>Values<td> IonExit | IonStop | Vacancy | Replacement | CascadeComplete | BoundaryCrossing
<tr><td>Default Value<td>"IonStop"<tr><td>Description <td>Simulation event that will trigger a tally score.
<h4>Options</h4><ul><li><strong>IonExit</strong> - The ion exits the simulation volume</li><li><strong>IonStop</strong> - The ion stops inside the simulation volume</li><li><strong>Vacancy</strong> - A lattice vacancy is created</li><li><strong>Replacement</strong> - A replacement event occurs</li><li><strong>CascadeComplete</strong> - A PKA cascade is completed</li><li><strong>BoundaryCrossing</strong> - An ion crosses an internal cell boundary</li></ul><tr><th colspan="2">\anchor _UserTally_0_coordinate_system /UserTally/0/coordinate_system<tr><td>Label <td>UserTally coordinate system
<tr><td>Type <td>Option group
<tr><td>Description <td>Local coordinate system used to interpret the tally's bin coordinates.
<tr><th colspan="2">\anchor _UserTally_0_coordinate_system_origin /UserTally/0/coordinate_system/origin<tr><td>Label <td>Origin [x₀,y₀,z₀] (nm)
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the UserTally coordinates.
<h4>Notes</h4><ul><li>The vector is defined with respect to the simulation space.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_coordinate_system_zaxis /UserTally/0/coordinate_system/zaxis<tr><td>Label <td>Z-axis direction vector
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,1.0]<tr><td>Description <td>A vector parallel to the z-axis of the UserTally coordinate system.
<h4>Notes</h4><ul><li>The vector is defined with respect to the simulation space.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_coordinate_system_xzvector /UserTally/0/coordinate_system/xzvector<tr><td>Label <td>Vector on the xz-plane
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>3
<tr><td>Element range<td>-1e+07...1e+07
<tr><td>Default Value<td>[1.0,0.0,1.0]<tr><td>Description <td>A vector on the xz-plane of the UserTally coordinate system.
<h4>Notes</h4><ul><li>The vector is defined with respect to the simulation space.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins /UserTally/0/bins<tr><td>Label <td>Bin edges
<tr><td>Type <td>Option group
<tr><td>Description <td>Bin edge definitions for each variable scored by the tally.
<tr><th colspan="2">\anchor _UserTally_0_bins_x /UserTally/0/bins/x<tr><td>Label <td>x-coordinate bin edges [nm]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion position x-coordinate.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_y /UserTally/0/bins/y<tr><td>Label <td>y-coordinate bin edges [nm]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion position y-coordinate.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_z /UserTally/0/bins/z<tr><td>Label <td>z-coordinate bin edges [nm]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1e+12...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion position z-coordinate.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_r /UserTally/0/bins/r<tr><td>Label <td>Radial distance r [nm]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's r=sqrt(x^2+y^2+z^2).
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_rho /UserTally/0/bins/rho<tr><td>Label <td>Cylindrical radial distance ρ [nm]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's ρ=sqrt(x^2+y^2).
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_cosTheta /UserTally/0/bins/cosTheta<tr><td>Label <td>Polar angle cosine cosθ
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1...1
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's position cosθ = z/r.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_nx /UserTally/0/bins/nx<tr><td>Label <td>x-axis direction cosine
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1...1
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's x-axis direction cosine.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_ny /UserTally/0/bins/ny<tr><td>Label <td>y-axis direction cosine
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1...1
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's y-axis direction cosine.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_nz /UserTally/0/bins/nz<tr><td>Label <td>z-axis direction cosine
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>-1...1
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's z-axis direction cosine.
<h4>Notes</h4><ul><li>Coordinates refer to the UserTally coordinate system.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_E /UserTally/0/bins/E<tr><td>Label <td>Ion kinetic energy [eV]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's kinetic energy.
<h4>Notes</h4><ul><li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_Tdam /UserTally/0/bins/Tdam<tr><td>Label <td>Ion damage energy [eV]
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's damage energy.
<h4>Notes</h4><ul><li>Tdam refers to the energy of the PKA dissipated to atomic displacements during the whole cascade.</li>
<li>This can be used only with events of type CascadeComplete.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_V /UserTally/0/bins/V<tr><td>Label <td>Number of generated vacancies
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+12
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the number of generated vacancies.
<h4>Notes</h4><ul><li>It refers to the total number of vacancies generated in a PKA cascade.</li>
<li>This can be used only with events of type CascadeComplete.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_atom_id /UserTally/0/bins/atom_id<tr><td>Label <td>Atomic species id
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+06
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's atomic species id.
<h4>Notes</h4><ul><li>Each atomic species in the simulation is assigned an id number.</li>
<li>Beam ions always have an id of 0.</li>
<li>Target atom recoils have id&gt;=1.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul><tr><th colspan="2">\anchor _UserTally_0_bins_recoil_id /UserTally/0/bins/recoil_id<tr><td>Label <td>Recoil generation id
<tr><td>Type <td>Vector of floating point values
<tr><td>Size<td>Variable
<tr><td>Element range<td>0...1e+06
<tr><td>Default Value<td>[]<tr><td>Description <td>Bin edges for the ion's recoil generation id.
<h4>Notes</h4><ul><li>Beam ions always have a recoil generation id of 0.</li>
<li>PKAs have id=1. Higher-order recoils have higher recoil ids.</li>
<li>Bin edges must be monotonously increasing.</li>
</ul></table>
