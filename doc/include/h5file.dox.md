## Archive structure

<table>
<caption>OpenTRIM HDF5 output archive structure</caption>
<tr><th>Path
<th>Type
<th>Size
<th>Description
<tr><td>/
<td>Group<td><td>File root object
<tr><td>&emsp;&emsp;run_info/
<td>Group<td><td>Simulation run information
<tr><td>&emsp;&emsp;&emsp;&emsp;title
<td>Text
<td>Scalar
<td>User supplied simulation title
<tr><td>&emsp;&emsp;&emsp;&emsp;total_ion_count
<td>Numeric
<td>Scalar
<td>Total number of simulated ion histories
<tr><td>&emsp;&emsp;&emsp;&emsp;json_config
<td>Text
<td>Scalar
<td>JSON formatted simulation options
<tr><td>&emsp;&emsp;&emsp;&emsp;version_info/
<td>Group<td><td>Program version information
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;name
<td>Text
<td>Scalar
<td>Code name
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;version
<td>Text
<td>Scalar
<td>Code version
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;git_tag
<td>Text
<td>Scalar
<td>Output of 'git describe --tags --always'
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;compiler
<td>Text
<td>Scalar
<td>C++ compiler
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;compiler_version
<td>Text
<td>Scalar
<td>C++ compiler version
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;build_system
<td>Text
<td>Scalar
<td>System used for building the code
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;build_time
<td>Text
<td>Scalar
<td>Build time
<tr><td>&emsp;&emsp;&emsp;&emsp;run_history
<td>Text
<td>Scalar
<td>JSON formattet run history
<tr><td>&emsp;&emsp;&emsp;&emsp;rng_state
<td>Numeric
<td>[4]
<td>Random number generator state
<tr><td>&emsp;&emsp;Target/
<td>Group<td><td>Information about the simulated target structure
<tr><td>&emsp;&emsp;&emsp;&emsp;grid/
<td>Group<td><td>Description of the 3D spatial grid
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;X
<td>Numeric
<td>\f$[N_x]\f$
<td>X-axis grid
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Y
<td>Numeric
<td>\f$[N_y]\f$
<td>Y-axis grid
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Z
<td>Numeric
<td>\f$[N_z]\f$
<td>Z-axis grid
<tr><td>&emsp;&emsp;&emsp;&emsp;materials/
<td>Group<td><td>Target materials definition
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;name
<td>Text
<td>\f$[N_{mat}]\f$
<td>Name of the material
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;atomic_density
<td>Numeric
<td>\f$[N_{mat}]\f$
<td>Atomic density [at/nm^3]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;mass_density
<td>Numeric
<td>\f$[N_{mat}]\f$
<td>Mass density [at/nm^3]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;atomic_radius
<td>Numeric
<td>\f$[N_{mat}]\f$
<td>Atomic radius [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;atoms/
<td>Group<td><td>Properties of atoms
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;label
<td>Text
<td>\f$[N_{at}]\f$
<td>label = [Chemical symbol] in [Material]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;symbol
<td>Text
<td>\f$[N_{at}]\f$
<td>Chemical symbol
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Z
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Atomic number
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;M
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Atomic mass
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Ed
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Displacement energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;El
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Lattice Binding energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Es
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Surface binding energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Er
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Replacement energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Rc
<td>Numeric
<td>\f$[N_{at}]\f$
<td>Recombination radius [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;dedx/
<td>Group<td><td>Electronic stopping tables
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;erg
<td>Numeric
<td>\f$[N_{e}]\f$
<td>Stopping table energy grid [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;dEdx
<td>Numeric
<td>\f$[N_{at},N_{mat},N_e]\f$
<td>Electronic stopping power [eV/nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;strag
<td>Numeric
<td>\f$[N_{at},N_{mat},N_e]\f$
<td>Electronic straggling [eV/nm^1/2]
<tr><td>&emsp;&emsp;&emsp;&emsp;flight_path/
<td>Group<td><td>Flight path selection tables
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;erg
<td>Numeric
<td>\f$[N_{e}]\f$
<td>Flight path selection table energy grid [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;mfp
<td>Numeric
<td>\f$[N_{at},N_{mat},N_e]\f$
<td>Mean free path [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;ipmax
<td>Numeric
<td>\f$[N_{at},N_{mat},N_e]\f$
<td>Max impact parameter [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;fpmax
<td>Numeric
<td>\f$[N_{at},N_{mat},N_e]\f$
<td>Max flight path [nm]
<tr><td>&emsp;&emsp;ion_beam/
<td>Group<td><td>Ion source details
<tr><td>&emsp;&emsp;&emsp;&emsp;E0
<td>Numeric
<td>Scalar
<td>Mean ion beam energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;Z
<td>Numeric
<td>Scalar
<td>Atomic number of source ions
<tr><td>&emsp;&emsp;&emsp;&emsp;M
<td>Numeric
<td>Scalar
<td>Atomic mass of source ions
<tr><td>&emsp;&emsp;&emsp;&emsp;dir0
<td>Numeric
<td>[3]
<td>Mean direction of source ions
<tr><td>&emsp;&emsp;&emsp;&emsp;x0
<td>Numeric
<td>[3]
<td>Mean position of source ions
<tr><td>&emsp;&emsp;tally/
<td>Group<td><td>Standard tally tables
<tr><td>&emsp;&emsp;&emsp;&emsp;damage_events/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Implantations
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Implantations & Interstitials
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Recombinations
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Recombinations
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Replacements
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Replacements
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Vacancies
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Vacancies
<tr><td>&emsp;&emsp;&emsp;&emsp;energy_deposition/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Ionization
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Energy deposited to ionization [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Lattice
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Energy deposited to the lattice as thermal energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Lost
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Energy lost due to ions exiting the simulation [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Stored
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Energy stored in lattice defects [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;ion_stat/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Collisions
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Ion collisions
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Flight_path
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Flight path [nm]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Lost
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Ions that exit the simulation volume
<tr><td>&emsp;&emsp;&emsp;&emsp;pka_damage/
<td>Group<td><td>
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Pka
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Primary knock-on atoms (PKAs)
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Pka_energy
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>PKA recoil energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Tdam
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Damage energy [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Tdam_LSS
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Damage energy estimated by the LSS approximation [eV]
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Vnrt
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Vacancies per the NRT model using Tdam
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Vnrt_LSS
<td>Numeric
<td>\f$[N_{at},N_x,N_y,N_z]\f$
<td>Vacancies per the NRT model using Tdam_LSS
<tr><td>&emsp;&emsp;user_tally/
<td>Group<td><td>User defined tallies
<tr><td>&emsp;&emsp;&emsp;&emsp;tally name/
<td>Group<td><td>Name of user tally
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;description
<td>Text
<td>Scalar
<td>Tally description
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;coordinate_system/
<td>Group<td><td>User tally coordinate system definition
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;origin
<td>Numeric
<td>[3]
<td>Coordinate system origin
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;zaxis
<td>Numeric
<td>[3]
<td>Coordinate system z-axis direction
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;xzvector
<td>Numeric
<td>[3]
<td>Vector on the xz-plane
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event
<td>Text
<td>Scalar
<td>Tally event
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event_description
<td>Text
<td>Scalar
<td>Tally event description
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;bin_names
<td>Text
<td>\f$[N_b]\f$
<td>Bin names
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;bin_descriptions
<td>Text
<td>\f$[N_b]\f$
<td>Bin descriptions
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;data
<td>Numeric
<td>\f$[N_1,N_2,...,N_{N_b}]\f$
<td>Tally bin data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;data_sem
<td>Numeric
<td>\f$[N_1,N_2,...,N_{N_b}]\f$
<td>Standard error in the mean of tally bin data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;bins/
<td>Group<td><td>Bin edges
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;bin_name1
<td>Numeric
<td>\f$[N_1+1]\f$
<td>Bin edges of variable 1
<tr><td>&emsp;&emsp;events/
<td>Group<td><td>lists of simulation events
<tr><td>&emsp;&emsp;&emsp;&emsp;exit/
<td>Group<td><td>Ions that exit the simulation volume
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event_data
<td>Numeric
<td>\f$[N_{ev},N_{cols}]\f$
<td>Event data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_names
<td>Text
<td>\f$[N_{ev},N_{cols}]\f$
<td>Names of event data columns
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_descriptions
<td>Text
<td>\f$[N_{ev},N_{cols}]\f$
<td>Description of event data columns
<tr><td>&emsp;&emsp;&emsp;&emsp;pka/
<td>Group<td><td>PKA events
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event_data
<td>Numeric
<td>\f$[N_{ev},N_{cols}]\f$
<td>Event data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_names
<td>Text
<td>\f$[N_{ev},N_{cols}]\f$
<td>Names of event data columns
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_descriptions
<td>Text
<td>\f$[N_{ev},N_{cols}]\f$
<td>Description of event data columns
<tr><td>&emsp;&emsp;&emsp;&emsp;damage/
<td>Group<td><td>Damage events
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;event_data
<td>Numeric
<td>\f$[N_{ev},N_{cols}]\f$
<td>Event data
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_names
<td>Text
<td>\f$[N_{ev},N_{cols}]\f$
<td>Names of event data columns
<tr><td>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;column_descriptions
<td>Text
<td>\f$[N_{ev},N_{cols}]\f$
<td>Description of event data columns
</table>


