
## The JSON configuration file {#json_config}

All configuration parameters for a simulation are coded in a [JSON](https://www.json.org/json-el.html) formatted string. 

This can be loaded directly from a file to the \ref cliapp "opentrim cli program" with the following command:

    opentrim -f config.json

where `config.json` is a file containing the JSON configuration.

In a C++ program one can use the \ref mcdriver::options class which offers
a parseJSON() function to parse and validate the options.

The JSON-formatted options string has the following self-explanatory structure shown bellow.
Click on any of the options to see more information.

> <br>
{<br>
&emsp;&emsp;\ref _L0_ "\"Simulation\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _L1_ "\"simulation_type\"": "FullCascade",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L2_ "\"screening_type\"": "ZBL",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L3_ "\"scattering_calculation\"": "Corteo4bitTable",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L4_ "\"eloss_calculation\"": "EnergyLoss",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L5_ "\"straggling_model\"": "BohrStraggling",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L6_ "\"nrt_calculation\"": "NRT_element",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L7_ "\"intra_cascade_recombination\"": false<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _L8_ "\"Transport\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _L9_ "\"min_energy\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L10_ "\"flight_path_type\"": "AtomicSpacing",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L11_ "\"flight_path_const\"": 0.1,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L12_ "\"max_rel_eloss\"": 0.05,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L13_ "\"min_recoil_energy\"": 1.0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L14_ "\"max_mfp\"": 1e+30,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L15_ "\"allow_sub_ml_scattering\"": false<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _L16_ "\"IonBeam\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _L17_ "\"ion\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L18_ "\"symbol\"": "H",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L19_ "\"atomic_number\"": 1,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L20_ "\"atomic_mass\"": 1.007825<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _L21_ "\"energy_distribution\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L22_ "\"type\"": "SingleValue",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L23_ "\"center\"": 1e+06,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L24_ "\"fwhm\"": 1.0<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _L25_ "\"spatial_distribution\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L26_ "\"geometry\"": "Surface",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L27_ "\"type\"": "SingleValue",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L28_ "\"center\"": [0.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L29_ "\"fwhm\"": 1.0<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _L30_ "\"angular_distribution\"": {<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L31_ "\"type\"": "SingleValue",<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L32_ "\"center\"": [1.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;\ref _L33_ "\"fwhm\"": 1.0<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _L34_ "\"Target\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _L35_ "\"size\"": [100.0,100.0,100.0],<br>
&emsp;&emsp;&emsp;&emsp;\ref _L36_ "\"origin\"": [0.0,0.0,0.0],<br>
&emsp;&emsp;&emsp;&emsp;\ref _L37_ "\"cell_count\"": [1,1,1],<br>
&emsp;&emsp;&emsp;&emsp;\ref _L38_ "\"periodic_bc\"": [0,1,1],<br>
&emsp;&emsp;&emsp;&emsp;\ref _L39_ "\"materials\"": {<br>
<br>
&emsp;&emsp;&emsp;&emsp;},<br>
&emsp;&emsp;&emsp;&emsp;\ref _L40_ "\"regions\"": {<br>
<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _L41_ "\"Output\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _L42_ "\"title\"": "Ion Simulation",<br>
&emsp;&emsp;&emsp;&emsp;\ref _L43_ "\"storage_interval\"": 1000,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L44_ "\"store_exit_events\"": 0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L45_ "\"store_pka_events\"": 0,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L46_ "\"store_dedx\"": 1<br>
&emsp;&emsp;},<br>
&emsp;&emsp;\ref _L47_ "\"Driver\"": {<br>
&emsp;&emsp;&emsp;&emsp;\ref _L48_ "\"max_no_ions\"": 100,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L49_ "\"threads\"": 1,<br>
&emsp;&emsp;&emsp;&emsp;\ref _L50_ "\"seed\"": 123456789<br>
&emsp;&emsp;}<br>
}<br>
<br>


@note `opentrim` accepts comments in the JSON config string
                        
The easiest way to get started is to get a template with all default options by running
                                        
    opentrim -t > template.json
                        
and make changes to the new template file.
                
Most of the options shown above are default values and can be omitted.


<table>
<caption>Detailed Description</caption>
<tr><th colspan="2">\anchor _L0_ /Simulation
<tr><td>Type <td>Option group
<tr><td>Description <td>General simulation options
<tr><th colspan="2">\anchor _L1_ /Simulation/simulation_type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> FullCascade | IonsOnly | CascadesOnly
<tr><td>Default Value<td>"FullCascade"<tr><td>Description <td>Define the type of simulation.
<br>
- FullCascade: Simulate full damage cascades, follow recoils<br>
- IonsOnly: Simulate only ions, do not follow recoils<br>
- CascadesOnly: Simulate recoil cascades<br>
<tr><th colspan="2">\anchor _L2_ /Simulation/screening_type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> None | LenzJensen | KrC | Moliere | ZBL
<tr><td>Default Value<td>"ZBL"<tr><td>Description <td>Define the type of screening potential.
<br>
- None: Unscreened Coulomb potential<br>
- LenzJensen: Lenz-Jensen screening<br>
- KrC: Kr-C screening<br>
- Moliere - Moliere screening<br>
- ZBL - Ziegler-Biersack-Littmark universal screening<br>
<tr><th colspan="2">\anchor _L3_ /Simulation/scattering_calculation
<tr><td>Type <td>Enumerator
<tr><td>Values<td> Corteo4bitTable | ZBL_MAGICK
<tr><td>Default Value<td>"Corteo4bitTable"<tr><td>Description <td>Define how ion scattering is calculated.
<br>
- Corteo4bitTable: Using 4-bit Corteo interpolation tables<br>
- ZBL_MAGICK: Analytical calculation using ZBL's MAGICK formula<br>
<tr><th colspan="2">\anchor _L4_ /Simulation/eloss_calculation
<tr><td>Type <td>Enumerator
<tr><td>Values<td> EnergyLossOff | EnergyLoss | EnergyLossAndStraggling
<tr><td>Default Value<td>"EnergyLoss"<tr><td>Description <td>Setting for electronic energy loss calculation.
<br>
- EnergyLossOff: No energy loss calculation<br>
- EnergyLoss: Only energy loss is calculated<br>
- EnergyLossAndStraggling: Both energy loss and straggling are calculated<br>
<tr><th colspan="2">\anchor _L5_ /Simulation/straggling_model
<tr><td>Type <td>Enumerator
<tr><td>Values<td> BohrStraggling | ChuStraggling | YangStraggling
<tr><td>Default Value<td>"BohrStraggling"<tr><td>Description <td>Model used for electronic straggling calculations.
<tr><th colspan="2">\anchor _L6_ /Simulation/nrt_calculation
<tr><td>Type <td>Enumerator
<tr><td>Values<td> NRT_element | NRT_average
<tr><td>Default Value<td>"NRT_element"<tr><td>Description <td>Define how to implement NRT in multielemental targets.
<br>
- NRT_element: NRT calculated per recoil atom<br>
- NRT_average: NRT calculated using material average values<br>
<tr><th colspan="2">\anchor _L7_ /Simulation/intra_cascade_recombination
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Enable intra-cascade recombination of Frenkel pairs.
<tr><th colspan="2">\anchor _L8_ /Transport
<tr><td>Type <td>Option group
<tr><td>Description <td>Ion transport options
<tr><th colspan="2">\anchor _L9_ /Transport/min_energy
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Lowest kinetic energy of a simulated ion.
<br>
When the energy of an ion goes below this cutoff,<br>
the ion history is terminated.<br>
<tr><th colspan="2">\anchor _L10_ /Transport/flight_path_type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> AtomicSpacing | Constant | MendenhallWeller | FullMC
<tr><td>Default Value<td>"AtomicSpacing"<tr><td>Description <td>Flight path selection algorithm.
<br>
- AtomicSpacing: Constant flight path equal to material's interatomic distance<br>
- Constant: User defined constant flight path<br>
- MendenhallWeller: SRIM-like path selection algorithm<br>
- FullMC: Full Monte-Carlo flight path algorithm<br>
<tr><th colspan="2">\anchor _L11_ /Transport/flight_path_const
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>0.1<tr><td>Description <td>Constant Flight Path in nm.
<br>
Used when flight_path_type=Constant<br>
<tr><th colspan="2">\anchor _L12_ /Transport/max_rel_eloss
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1
<tr><td>Default Value<td>0.05<tr><td>Description <td>Maximum allowed relative energy loss per flight path.
<br>
Applicable only when flight_path_type=MendenhallWeller or FullMC<br>
<tr><th colspan="2">\anchor _L13_ /Transport/min_recoil_energy
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+06
<tr><td>Default Value<td>1.0<tr><td>Description <td>Minimum recoil energy in eV.
<br>
Applicable only when flight_path_type=MendenhallWeller or FullMC<br>
<tr><th colspan="2">\anchor _L14_ /Transport/max_mfp
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1e+30
<tr><td>Default Value<td>1e+30<tr><td>Description <td>Maximum ion mean free path in nm.
<br>
Applicable only when flight_path_type=FullMC<br>
<tr><th colspan="2">\anchor _L15_ /Transport/allow_sub_ml_scattering
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>false<tr><td>Description <td>Allow sub-monolayer ion flight path.
<br>
Applicable only when flight_path_type=FullMC<br>
<tr><th colspan="2">\anchor _L16_ /IonBeam
<tr><td>Type <td>Option group
<tr><td>Description <td>Ion Source
<tr><th colspan="2">\anchor _L17_ /IonBeam/ion
<tr><td>Type <td>Option group
<tr><td>Description <td>Projectile ion definition
<tr><th colspan="2">\anchor _L18_ /IonBeam/ion/symbol
<tr><td>Type <td>String
<tr><td>Default Value<td>"H"<tr><td>Description <td>Chemical element symbol.
<tr><th colspan="2">\anchor _L19_ /IonBeam/ion/atomic_number
<tr><td>Type <td>Integer
<tr><td>Range<td>1...92
<tr><td>Default Value<td>1<tr><td>Description <td>Atomic number of the generated ions.
<tr><th colspan="2">\anchor _L20_ /IonBeam/ion/atomic_mass
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.001...1000
<tr><td>Default Value<td>1.007825<tr><td>Description <td>Atomic mass of the generated ions.
<tr><th colspan="2">\anchor _L21_ /IonBeam/energy_distribution
<tr><td>Type <td>Option group
<tr><td>Description <td>Energy Distribution
<tr><th colspan="2">\anchor _L22_ /IonBeam/energy_distribution/type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of energy distribution of the generated ions.
<br>
- Single Value: All ions have the same energy<br>
- Uniform: Ion energy distributed uniformly within center ± fwhm/2<br>
- Gaussian: Ion energy distributed according to the Gaussian(Normal) distribution around the center value with given fwhm<br>
 <br>
When sampling from a distribution, out-of-bounds values are rejected and a new sample is drawn.<br>
<tr><th colspan="2">\anchor _L23_ /IonBeam/energy_distribution/center
<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1e+06<tr><td>Description <td>Central energy of generated ions in eV.
<tr><th colspan="2">\anchor _L24_ /IonBeam/energy_distribution/fwhm
<tr><td>Type <td>Floating point number
<tr><td>Range<td>1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions energy distribution in eV.
<tr><th colspan="2">\anchor _L25_ /IonBeam/spatial_distribution
<tr><td>Type <td>Option group
<tr><td>Description <td>Spatial Distribution
<tr><th colspan="2">\anchor _L26_ /IonBeam/spatial_distribution/geometry
<tr><td>Type <td>Enumerator
<tr><td>Values<td> Surface | Volume
<tr><td>Default Value<td>"Surface"<tr><td>Description <td>Geometry of the ion source.
<br>
- Surface: Ions are generated on a simulation boundary surface.<br>
- Volume: Ions are generated within the simulation volume.<br>
<tr><th colspan="2">\anchor _L27_ /IonBeam/spatial_distribution/type
<tr><td>Type <td>Enumerator
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
<tr><th colspan="2">\anchor _L28_ /IonBeam/spatial_distribution/center
<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>-1e+10...1e+10
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Central initial position of generated ions, [x,y,z] in nm.
<tr><th colspan="2">\anchor _L29_ /IonBeam/spatial_distribution/fwhm
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.1...1e+10
<tr><td>Default Value<td>1.0<tr><td>Description <td>Full-width at half-maximum of the generated ions position distribution in nm.
<tr><th colspan="2">\anchor _L30_ /IonBeam/angular_distribution
<tr><td>Type <td>Option group
<tr><td>Description <td>Angular Distribution
<tr><th colspan="2">\anchor _L31_ /IonBeam/angular_distribution/type
<tr><td>Type <td>Enumerator
<tr><td>Values<td> SingleValue | Uniform | Gaussian
<tr><td>Default Value<td>"SingleValue"<tr><td>Description <td>Type of angular distribution of the generated ions.
<br>
- Single Value: All ions have the same initial direction<br>
- Uniform: Ion direction distributed uniformly within a cone around the central direction<br>
- Gaussian: Ion direction distributed according to the Gaussian(Normal) distribution around the central direction<br>
<tr><th colspan="2">\anchor _L32_ /IonBeam/angular_distribution/center
<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>-1000...1000
<tr><td>Default Value<td>[1.0,0.0,0.0]<tr><td>Description <td>Ion beam central direction vector, [nx,ny,nz], unnormalized.
<tr><th colspan="2">\anchor _L33_ /IonBeam/angular_distribution/fwhm
<tr><td>Type <td>Floating point number
<tr><td>Range<td>0.01...1000
<tr><td>Default Value<td>1.0<tr><td>Description <td>Width in srad of a cone around the central ion beam direction.
<br>
For the Uniform distribution, fwhm defines a cone around the main direction, where the direction of generated ions is sampled uniformly<br>
The Gaussian distrubution is not yet implemented<br>
<tr><th colspan="2">\anchor _L34_ /Target
<tr><td>Type <td>Option group
<tr><td>Description <td>Target
<tr><th colspan="2">\anchor _L35_ /Target/size
<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>0.001...1e+07
<tr><td>Default Value<td>[100.0,100.0,100.0]<tr><td>Description <td>Size in nm of the simulation volume along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _L36_ /Target/origin
<tr><td>Type <td>Floating point 3d Vector
<tr><td>Range<td>-1e+07...1e+07
<tr><td>Default Value<td>[0.0,0.0,0.0]<tr><td>Description <td>Origin of the simulation space.
<tr><th colspan="2">\anchor _L37_ /Target/cell_count
<tr><td>Type <td>Integer 3d Vector
<tr><td>Range<td>1...1e+06
<tr><td>Default Value<td>[1,1,1]<tr><td>Description <td>Number of simulation cells along the x-, y- and z-axis.
<tr><th colspan="2">\anchor _L38_ /Target/periodic_bc
<tr><td>Type <td>Integer 3d Vector
<tr><td>Range<td>0...1
<tr><td>Default Value<td>[0,1,1]<tr><td>Description <td>Select periodic boundary conditions along the axes (0=normal, 1=periodic).
<tr><th colspan="2">\anchor _L39_ /Target/materials
<tr><td>Type <td>Option group
<tr><td>Description <td>Target materials definition
<tr><th colspan="2">\anchor _L40_ /Target/regions
<tr><td>Type <td>Option group
<tr><td>Description <td>Target regions definition
<tr><th colspan="2">\anchor _L41_ /Output
<tr><td>Type <td>Option group
<tr><td>Description <td>Output
<tr><th colspan="2">\anchor _L42_ /Output/title
<tr><td>Type <td>String
<tr><td>Default Value<td>"Ion Simulation"<tr><td>Description <td>Short title describing the simulation.
<tr><th colspan="2">\anchor _L43_ /Output/storage_interval
<tr><td>Type <td>Integer
<tr><td>Range<td>100...2.14748e+09
<tr><td>Default Value<td>1000<tr><td>Description <td>Time interval to update stored data.
<tr><th colspan="2">\anchor _L44_ /Output/store_exit_events
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>0<tr><td>Description <td>Store a table with ions that exit the simulation.
<tr><th colspan="2">\anchor _L45_ /Output/store_pka_events
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>0<tr><td>Description <td>Store a table of PKA data.
<tr><th colspan="2">\anchor _L46_ /Output/store_dedx
<tr><td>Type <td>Boolean
<tr><td>Default Value<td>1<tr><td>Description <td>Store stopping tables for each ion/material combination.
<tr><th colspan="2">\anchor _L47_ /Driver
<tr><td>Type <td>Option group
<tr><td>Description <td>Driver
<tr><th colspan="2">\anchor _L48_ /Driver/max_no_ions
<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>100<tr><td>Description <td>Maximum number of histories to simulate.
<tr><th colspan="2">\anchor _L49_ /Driver/threads
<tr><td>Type <td>Integer
<tr><td>Range<td>1...1000
<tr><td>Default Value<td>1<tr><td>Description <td>Number of execution threads.
<tr><th colspan="2">\anchor _L50_ /Driver/seed
<tr><td>Type <td>Integer
<tr><td>Range<td>1...2.14748e+09
<tr><td>Default Value<td>123456789<tr><td>Description <td>Random number generator seed.
</table>
