
## The HDF5 output archive {#out_file}

The simulation produces a single HDF5 output file which contains all
results from tallies and events along with the input conditions, run
timing and other information. 
 
A brief description of the file contents is given here. More detailed
information can be found within the file itself. 

Dimensions of tables depend on:
- \f$N_{at}\f$ : # of atoms, i.e., all atoms in the target plus the
projectile. Note that the same atom species belonging to different
materials is counted as different atom. \n
The order of atoms can be seen in `/target/atoms/labels`.
- \f$N_{mat}\f$ : # of target materials
- \f$N_x, N_y, N_z\f$ : # of grid points along the 3 axes
- \f$N_c=(N_x-1)(N_y-1)(N_z-1)\f$ : # of target cells
- \f$N_e\f$ : # of energy points for energy loss tables
- \f$N_{ev}\f$ : # of events

To reach a variable in the archive use the complete path, e.g. `/tally/energy_deposition/Ionization`.

<table>
<caption>HDF5 output archive structure</caption>
<tr><th>Path
<th>Type
<th>Size
<th>Description
<tr><td>/
<td>Group<td><td>
<tr><td>&emsp;&emsp;run_info/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;title
<td>String
<td>Scalar
<td>User supplied simulation title
<tr><td>&emsp;&emsp;&emsp;&emsp;total_ion_count
<td>Numeric
<td>Scalar
<td>Total number of simulated ions
<tr><td>&emsp;&emsp;&emsp;&emsp;config_json
<td>String
<td>Scalar
<td>JSON formatted simulation options
<tr><td>&emsp;&emsp;&emsp;&emsp;variable_list
<td>String
<td>Scalar
<td>List of all data variables in the file
<tr><td>&emsp;&emsp;&emsp;&emsp;version_info/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;name
<td>String
<td>Scalar
<td>program name
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;version
<td>String
<td>Scalar
<td>version
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;compiler
<td>String
<td>Scalar
<td>compiler
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;compiler_version
<td>String
<td>Scalar
<td>compiler version
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;build_system
<td>String
<td>Scalar
<td>build system
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;build_time
<td>String
<td>Scalar
<td>build time
<tr><td>&emsp;&emsp;&emsp;&emsp;run_history
<td>String
<td>Scalar
<td>run history
<tr><td>&emsp;&emsp;&emsp;&emsp;rng_state
<td>Numeric
<td>4
<td>random generator state
<tr><td>&emsp;&emsp;target/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;grid/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;name
<td>String
<td>Scalar
<td>program name
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;X
<td>Numeric
<td>Nx
<td>x-axis grid
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Y
<td>Numeric
<td>Nx
<td>y-axis grid
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Z
<td>Numeric
<td>Nx
<td>z-axis grid
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;cell_xyz
<td>Numeric
<td>Nc x 3
<td>Cell center coordinates
<tr><td>&emsp;&emsp;&emsp;&emsp;atoms/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;label
<td>String
<td>Nat
<td>labels = [Atom (Chemical symbol)] in [Material]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;symbol
<td>String
<td>Nat
<td>Chemical symbol
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Z
<td>Numeric
<td>Nat
<td>Atomic number
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;M
<td>Numeric
<td>Nat
<td>Atomic mass
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Ed
<td>Numeric
<td>Nat
<td>Displacement energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;El
<td>Numeric
<td>Nat
<td>Lattice binding energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Es
<td>Numeric
<td>Nat
<td>Surface binding energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Er
<td>Numeric
<td>Nat
<td>Replacement energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;materials/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;name
<td>String
<td>Nmat
<td>name of material
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;atomic_density
<td>Numeric
<td>Nmat
<td>atomic density [at/nm^3]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;mass_density
<td>Numeric
<td>Nmat
<td>mass density [at/nm^3]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;atomic_radius
<td>Numeric
<td>Nmat
<td>atomic radius [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;dedx/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;erg
<td>Numeric
<td>Ne
<td>dEdx table energy grid [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;eloss
<td>Numeric
<td>Nat x Nmat x Ne
<td>dEdx values [eV/nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;strag
<td>Numeric
<td>Nat x Nmat x Ne
<td>straggling values [eV/nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;flight_path/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;erg
<td>Numeric
<td>Ne
<td>flight path table energy grid [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;mfp
<td>Numeric
<td>Nat x Nmat x Ne
<td>mean free path [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Tcutoff
<td>Numeric
<td>Nat x Nmat x Ne
<td>recoil energy cut0ff [eV]
<tr><td>&emsp;&emsp;ion_beam/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;E0
<td>Numeric
<td>Scalar
<td>mean energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;Z
<td>Numeric
<td>Scalar
<td>atomic number
<tr><td>&emsp;&emsp;&emsp;&emsp;M
<td>Numeric
<td>Scalar
<td>mass [amu]
<tr><td>&emsp;&emsp;&emsp;&emsp;dir0
<td>Numeric
<td>3
<td>mean direction
<tr><td>&emsp;&emsp;&emsp;&emsp;x0
<td>Numeric
<td>3
<td>mean position [nm]
<tr><td>&emsp;&emsp;tally/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;damage_events/Vacancies
<td>Numeric
<td>Nat x Nc
<td>Vacancies
<tr><td>&emsp;&emsp;&emsp;&emsp;damage_events/Implantations
<td>Numeric
<td>Nat x Nc
<td>Implantations & Interstitials
<tr><td>&emsp;&emsp;&emsp;&emsp;damage_events/Replacements
<td>Numeric
<td>Nat x Nc
<td>Replacements
<tr><td>&emsp;&emsp;&emsp;&emsp;damage_events/Recombinations
<td>Numeric
<td>Nat x Nc
<td>Intra-cascade recombinations
<tr><td>&emsp;&emsp;&emsp;&emsp;damage_events/Displacements
<td>Numeric
<td>Nat x Nc
<td>Displacements
<tr><td>&emsp;&emsp;&emsp;&emsp;energy_deposition/Ionization
<td>Numeric
<td>Nat x Nc
<td>Energy deposited to ionization [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;energy_deposition/Lattice
<td>Numeric
<td>Nat x Nc
<td>Energy deposited to the lattice as thermal energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;energy_deposition/Stored
<td>Numeric
<td>Nat x Nc
<td>Energy stored in lattice defects [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;energy_deposition/Lost
<td>Numeric
<td>Nat x Nc
<td>Energy lost due to ions exiting the simulation [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/Pka
<td>Numeric
<td>Nat x Nc
<td>Primary knock-on atoms (PKAs)
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/Pka_energy
<td>Numeric
<td>Nat x Nc
<td>PKA recoil energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/Tdam
<td>Numeric
<td>Nat x Nc
<td>Damage energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/Tdam_LSS
<td>Numeric
<td>Nat x Nc
<td>Damage energy estimated by the LSS approximation [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/Vnrt
<td>Numeric
<td>Nat x Nc
<td>Vacancies per the NRT model using Tdam
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/Vnrt_LSS
<td>Numeric
<td>Nat x Nc
<td>Vacancies per the NRT model using Tdam_LSS
<tr><td>&emsp;&emsp;&emsp;&emsp;ion_stat/Flight_path
<td>Numeric
<td>Nat x Nc
<td>Flight path [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;ion_stat/Collisions
<td>Numeric
<td>Nat x Nc
<td>Ion collisions
<tr><td>&emsp;&emsp;&emsp;&emsp;ion_stat/Lost
<td>Numeric
<td>Nat x Nc
<td>Ions that exit the simulation volume
<tr><td>&emsp;&emsp;&emsp;&emsp;totals/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;data
<td>Numeric
<td>19 x Nat
<td>tally totals per atom
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_names
<td>String
<td>19
<td>names of totals
<tr><td>&emsp;&emsp;events/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;pka/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event_data
<td>Numeric
<td>Nev x 10
<td>event data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_names
<td>String
<td>10
<td>event data column names
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_descriptions
<td>String
<td>10
<td>event data column descriptions
<tr><td>&emsp;&emsp;&emsp;&emsp;exit/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event_data
<td>Numeric
<td>Nev x 10
<td>event data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_names
<td>String
<td>10
<td>event data column names
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_descriptions
<td>String
<td>10
<td>event data column descriptions
</table>





