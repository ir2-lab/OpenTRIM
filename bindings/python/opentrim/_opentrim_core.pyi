"""
Python bindings for the OpenTRIM Monte Carlo ion transport simulator
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['AngularDistribution', 'Atom', 'AtomList', 'Config', 'CoordSys', 'Distribution', 'Driver', 'Element', 'EnergyDistribution', 'Event', 'FlightPath', 'Geometry', 'Info', 'IonBeamParams', 'Material', 'MaterialList', 'NRT_Impl', 'OutputOptions', 'Region', 'RegionList', 'RunOptions', 'ScreeningType', 'SimulationParams', 'SimulationType', 'SpatialDistribution', 'Stopping', 'Straggling', 'TargetParams', 'TransportParams', 'UserTally', 'UserTallyBins', 'UserTallyList']
class AngularDistribution:
    """
    Ion beam angular distribution.  Access via config.IonBeam.angular_distribution.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def center(self) -> list[float]:
        """
        Mean beam direction (unnormalized).  Default: [1, 0, 0] = along +x.
        """
    @center.setter
    def center(self, arg1: list[float]) -> None:
        ...
    @property
    def fwhm(self) -> float:
        """
        Angular spread FWHM [sr].
        """
    @fwhm.setter
    def fwhm(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def type(self) -> Distribution:
        """
        Angular distribution type (Distribution).
        """
    @type.setter
    def type(self, arg0: Distribution) -> None:
        ...
class Atom:
    """
    Target atom species parameters.
    
    Used in Material.composition to define each element in the material::
    
        atom = opentrim.Atom()
        atom.element = opentrim.Element("Fe")
        atom.X  = 1.0    # atomic fraction
        atom.Ed = 40.0   # displacement threshold [eV]
        atom.El = 3.0    # lattice binding energy [eV]
        atom.Es = 4.28   # surface binding energy [eV]
        atom.Er = 40.0   # replacement threshold [eV]
        atom.Rc = 0.946  # recombination radius [nm]
    """
    element: Element
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def Ed(self) -> float:
        """
        Displacement threshold energy [eV].
        """
    @Ed.setter
    def Ed(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def El(self) -> float:
        """
        Lattice binding energy [eV].
        """
    @El.setter
    def El(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def Er(self) -> float:
        """
        Replacement threshold energy [eV].
        """
    @Er.setter
    def Er(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def Es(self) -> float:
        """
        Surface binding energy [eV].
        """
    @Es.setter
    def Es(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def Rc(self) -> float:
        """
        Frenkel pair recombination radius [nm].
        """
    @Rc.setter
    def Rc(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def X(self) -> float:
        """
        Atomic fraction (sum over composition = 1).
        """
    @X.setter
    def X(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class AtomList:
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    @typing.overload
    def __delitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, s: slice) -> AtomList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Atom:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: AtomList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: collections.abc.Iterable) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator[Atom]:
        ...
    def __len__(self) -> int:
        ...
    @typing.overload
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: Atom) -> None:
        ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: AtomList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: Atom) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    @typing.overload
    def extend(self, L: AtomList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: collections.abc.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: typing.SupportsInt | typing.SupportsIndex, x: Atom) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> Atom:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: typing.SupportsInt | typing.SupportsIndex) -> Atom:
        """
        Remove and return the item at index ``i``
        """
class Config:
    """
    Full simulation configuration.
    
    Construction::
    
        config = opentrim.Config()            # all options at defaults
        config = opentrim.Config("sim.json")  # load from JSON file
    
    Attribute access (recommended -- IDE autocomplete works)::
    
        config.Simulation.electronic_stopping = opentrim.Stopping.SRIM13
        config.Run.max_no_ions = 10000
    
    Dict-style access (same fields, useful for programmatic access)::
    
        config["Run"]["threads"] = 4
        config["Simulation"]["electronic_stopping"] = opentrim.Stopping.SRIM13
    
    Methods::
    
        config.validate()                # raises ValueError on invalid config
        config.to_json()                 # -> str, full config as JSON
        opentrim.Config.from_json(s)     # classmethod: construct from JSON string
        opentrim.Config.options_spec()   # -> str, JSON spec of all options
    """
    IonBeam: IonBeamParams
    Output: OutputOptions
    Run: RunOptions
    Simulation: SimulationParams
    Target: TargetParams
    Transport: TransportParams
    @staticmethod
    def from_json(json_string: str) -> Config:
        """
        Construct a Config from a JSON string.  Does not validate; call config.validate() before use.
        """
    @staticmethod
    def options_spec() -> str:
        """
        Return the JSON spec of all config options (types, ranges, descriptions).
        """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    @typing.overload
    def __init__(self) -> None:
        """
        Create a Config with all options at their default values.
        """
    @typing.overload
    def __init__(self, filename: str) -> None:
        """
        Load a Config from a JSON file.  Raises ValueError on read or parse error.
        """
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    def to_json(self) -> str:
        """
        Return the full configuration as a JSON string.
        """
    def validate(self) -> None:
        """
        Validate the configuration.  Raises ValueError with a description if invalid.
        """
    @property
    def UserTally(self) -> UserTallyList:
        """
        List of user-defined tallies (UserTallyList).
        """
    @UserTally.setter
    def UserTally(self, arg0: UserTallyList) -> None:
        ...
class CoordSys:
    """
    Coordinate system used to orient a UserTally.
    
    Defines a local frame by a z-axis direction and a vector in the xz-plane.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def origin(self) -> list[float]:
        """
        Origin of the coordinate system [nm].
        """
    @origin.setter
    def origin(self, arg1: list[float]) -> None:
        ...
    @property
    def xzvector(self) -> list[float]:
        """
        A vector in the xz-plane used to define the x-axis.
        """
    @xzvector.setter
    def xzvector(self, arg1: list[float]) -> None:
        ...
    @property
    def zaxis(self) -> list[float]:
        """
        Z-axis direction vector (unnormalized).
        """
    @zaxis.setter
    def zaxis(self, arg1: list[float]) -> None:
        ...
class Distribution:
    """
    Statistical distribution for ion beam energy, position, and direction.
    
    Members:
    
      SingleValue : Delta distribution (default).
    
      Uniform
    
      Gaussian : Normal distribution.
    """
    Gaussian: typing.ClassVar[Distribution]  # value = <Distribution.Gaussian: 2>
    SingleValue: typing.ClassVar[Distribution]  # value = <Distribution.SingleValue: 0>
    Uniform: typing.ClassVar[Distribution]  # value = <Distribution.Uniform: 1>
    __members__: typing.ClassVar[dict[str, Distribution]]  # value = {'SingleValue': <Distribution.SingleValue: 0>, 'Uniform': <Distribution.Uniform: 1>, 'Gaussian': <Distribution.Gaussian: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Driver:
    """
    Runs an OpenTRIM simulation.  Wraps the C++ mcdriver.
    
    Example::
    
        sim = opentrim.Driver()
        sim.init(config)
        sim.run()          # Mode A - non-blocking
        sim.wait()
    """
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def abort(self) -> None:
        """
        Signal a running simulation to stop gracefully.
        """
    def config(self) -> Config:
        """
        Return a copy of the current configuration as a Config.
        """
    def init(self, config: Config) -> None:
        """
        Initialize the driver from a Config.  Resets any current simulation.
        """
    def ion_count(self) -> int:
        """
        Ions simulated so far.  Delegates to mccore::ion_count(); 0 if no
        simulation has been created yet.
        """
    def is_running(self) -> bool:
        """
        True while a simulation is active.  Tracked by the binding, not by
        mcdriver::is_running() (which is unreliable during setup).
        """
    def load(self, filename: str) -> None:
        """
        Load a simulation from an HDF5 file, replacing any current state.
        Afterwards results are available via Info(sim) and the run can be
        continued with run().
        """
    def reset(self) -> None:
        """
        Abort the running simulation and clear all simulation state.
        """
    @typing.overload
    def run(self) -> None:
        """
        Mode A: start the simulation and return immediately.  exec() runs on
        a background thread; use wait() to block until it finishes.
        """
    @typing.overload
    def run(self, callback: collections.abc.Callable, interval_ms: typing.SupportsInt | typing.SupportsIndex = 1000) -> None:
        """
        Mode B: run the simulation on the calling thread and call callback()
        every interval_ms milliseconds.  The GIL is released during the run.
        interval_ms is quantized to 100 ms ticks (minimum effective 100 ms).
        """
    def save(self, filename: str) -> None:
        """
        Save the simulation and all results to an HDF5 file.  Same format as
        the CLI output; reload it with load().  Requires at least one
        simulated ion (tallies are normalized per ion).
        """
    def wait(self) -> None:
        """
        Block until the Mode A run thread finishes.  No-op otherwise.
        """
    @property
    def max_cpu_time(self) -> int:
        """
        Maximum wall-clock time [s].  0 = unlimited.
        """
    @max_cpu_time.setter
    def max_cpu_time(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def max_ions(self) -> int:
        """
        Maximum ions to simulate (config.Run.max_no_ions).  Read once at
        exec() start; changes take effect on the next run().
        """
    @max_ions.setter
    def max_ions(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class Element:
    """
    Atomic element used to specify ion beam species and target atom species.
    
    Construction::
    
        opentrim.Element("He")  # from symbol, auto-fills Z=2, M=4.003
        opentrim.Element(2)     # from atomic number, auto-fills symbol and M
    
    Attributes::
    
        symbol        str   element symbol
        atomic_number int   atomic number Z
        atomic_mass   float mean atomic mass [amu]
    """
    symbol: str
    def __eq__(self, arg0: Element) -> bool:
        ...
    def __hash__(self) -> int:
        ...
    @typing.overload
    def __init__(self, symbol: str) -> None:
        """
        Construct from element symbol.  Raises ValueError for unknown symbols.
        """
    @typing.overload
    def __init__(self, Z: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Construct from atomic number Z (1-92).  Raises ValueError for out-of-range Z.
        """
    def __repr__(self) -> str:
        ...
    @property
    def atomic_mass(self) -> float:
        ...
    @atomic_mass.setter
    def atomic_mass(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def atomic_number(self) -> int:
        ...
    @atomic_number.setter
    def atomic_number(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class EnergyDistribution:
    """
    Ion beam energy distribution.  Access via config.IonBeam.energy_distribution.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def center(self) -> float:
        """
        Mean energy [eV].
        """
    @center.setter
    def center(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def fwhm(self) -> float:
        """
        FWHM of energy distribution [eV].
        """
    @fwhm.setter
    def fwhm(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def type(self) -> Distribution:
        """
        Distribution type (Distribution).
        """
    @type.setter
    def type(self, arg0: Distribution) -> None:
        ...
class Event:
    """
    Monte Carlo simulation events.  Used to configure UserTally triggers.
    
    Members:
    
      NewSourceIon
    
      NewRecoil
    
      Scattering
    
      IonExit
    
      IonStop : Default UserTally trigger.
    
      BoundaryCrossing
    
      Replacement
    
      Vacancy
    
      CascadeComplete
    
      NewFlightPath
    
      NEvent : Sentinel -- total event count.
    
      Invalid
    """
    BoundaryCrossing: typing.ClassVar[Event]  # value = <Event.BoundaryCrossing: 32>
    CascadeComplete: typing.ClassVar[Event]  # value = <Event.CascadeComplete: 256>
    Invalid: typing.ClassVar[Event]  # value = <Event.Invalid: -1>
    IonExit: typing.ClassVar[Event]  # value = <Event.IonExit: 8>
    IonStop: typing.ClassVar[Event]  # value = <Event.IonStop: 16>
    NEvent: typing.ClassVar[Event]  # value = <Event.NEvent: 1024>
    NewFlightPath: typing.ClassVar[Event]  # value = <Event.NewFlightPath: 512>
    NewRecoil: typing.ClassVar[Event]  # value = <Event.NewRecoil: 2>
    NewSourceIon: typing.ClassVar[Event]  # value = <Event.NewSourceIon: 1>
    Replacement: typing.ClassVar[Event]  # value = <Event.Replacement: 64>
    Scattering: typing.ClassVar[Event]  # value = <Event.Scattering: 4>
    Vacancy: typing.ClassVar[Event]  # value = <Event.Vacancy: 128>
    __members__: typing.ClassVar[dict[str, Event]]  # value = {'NewSourceIon': <Event.NewSourceIon: 1>, 'NewRecoil': <Event.NewRecoil: 2>, 'Scattering': <Event.Scattering: 4>, 'IonExit': <Event.IonExit: 8>, 'IonStop': <Event.IonStop: 16>, 'BoundaryCrossing': <Event.BoundaryCrossing: 32>, 'Replacement': <Event.Replacement: 64>, 'Vacancy': <Event.Vacancy: 128>, 'CascadeComplete': <Event.CascadeComplete: 256>, 'NewFlightPath': <Event.NewFlightPath: 512>, 'NEvent': <Event.NEvent: 1024>, 'Invalid': <Event.Invalid: -1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class FlightPath:
    """
    Free-flight path selection algorithm.
    
    Members:
    
      Constant : Constant MFP (default). Value: Transport.flight_path_const [atomic radii].
    
      Variable : Energy-dependent MFP sampled from pre-computed tables.
    """
    Constant: typing.ClassVar[FlightPath]  # value = <FlightPath.Constant: 0>
    Variable: typing.ClassVar[FlightPath]  # value = <FlightPath.Variable: 1>
    __members__: typing.ClassVar[dict[str, FlightPath]]  # value = {'Constant': <FlightPath.Constant: 0>, 'Variable': <FlightPath.Variable: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Geometry:
    """
    Ion source injection geometry.
    
    Members:
    
      Surface : Ions start at target surface (default).
    
      Volume : Ions start inside target volume.
    """
    Surface: typing.ClassVar[Geometry]  # value = <Geometry.Surface: 0>
    Volume: typing.ClassVar[Geometry]  # value = <Geometry.Volume: 1>
    __members__: typing.ClassVar[dict[str, Geometry]]  # value = {'Surface': <Geometry.Surface: 0>, 'Volume': <Geometry.Volume: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Info:
    """
    Read-only view of simulation results.  Wraps the C++ mcinfo tree.
    
    Example::
    
        info = opentrim.Info(sim)            # sim is an initialized Driver
        v, dv = info["tally"]["damage_events"]["Vacancies"]
    
    Tally nodes return (values, sem) numpy tuples normalized per ion.
    Note: reading results while a Mode A run is in progress may race;
    prefer calling sim.wait() first.
    """
    @staticmethod
    def info_spec() -> str:
        """
        Return the JSON spec of the full info tree structure.
        """
    def __contains__(self, arg0: str) -> bool:
        ...
    def __getitem__(self, key: str) -> typing.Any:
        """
        Return a sub-Info for a group, or the value for a leaf node.
        """
    def __init__(self, driver: typing.Any) -> None:
        """
        Build an Info view over an initialized Driver.
        """
    def __iter__(self) -> collections.abc.Iterator:
        ...
    def __len__(self) -> int:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def _repr_html_(self) -> str:
        ...
    def keys(self) -> list[str]:
        """
        Available keys at this level, in definition order.
        """
    @property
    def description(self) -> str:
        """
        Human-readable description of this node.
        """
    @property
    def type(self) -> str:
        """
        Node type: group, string, json, real64, real32, uint64, or tally_score.
        """
class IonBeamParams:
    """
    Ion beam source definition.  Access via config.IonBeam.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def angular_distribution(self) -> AngularDistribution:
        """
        Angular distribution (AngularDistribution).
        """
    @angular_distribution.setter
    def angular_distribution(self, arg0: AngularDistribution) -> None:
        ...
    @property
    def energy_distribution(self) -> EnergyDistribution:
        """
        Energy distribution (EnergyDistribution).
        """
    @energy_distribution.setter
    def energy_distribution(self, arg0: EnergyDistribution) -> None:
        ...
    @property
    def ion(self) -> Element:
        """
        Beam ion species (Element).
        """
    @ion.setter
    def ion(self, arg0: Element) -> None:
        ...
    @property
    def spatial_distribution(self) -> SpatialDistribution:
        """
        Spatial distribution (SpatialDistribution).
        """
    @spatial_distribution.setter
    def spatial_distribution(self, arg0: SpatialDistribution) -> None:
        ...
class Material:
    """
    Target material descriptor::
    
        mat = opentrim.Material()
        mat.id      = "Iron"
        mat.density = 7.874   # g/cm3
        mat.composition.append(atom)
        config.Target.materials.append(mat)
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def color(self) -> str:
        """
        Hex color string for GUI display, e.g. '#55aaff'.
        """
    @color.setter
    def color(self, arg0: str) -> None:
        ...
    @property
    def composition(self) -> AtomList:
        """
        List of Atom objects (AtomList).
        """
    @composition.setter
    def composition(self, arg0: AtomList) -> None:
        ...
    @property
    def density(self) -> float:
        """
        Mass density [g/cm3].
        """
    @density.setter
    def density(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def id(self) -> str:
        """
        User-defined material identifier.
        """
    @id.setter
    def id(self, arg0: str) -> None:
        ...
class MaterialList:
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    @typing.overload
    def __delitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, s: slice) -> MaterialList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Material:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: MaterialList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: collections.abc.Iterable) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator[Material]:
        ...
    def __len__(self) -> int:
        ...
    @typing.overload
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: Material) -> None:
        ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: MaterialList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: Material) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    @typing.overload
    def extend(self, L: MaterialList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: collections.abc.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: typing.SupportsInt | typing.SupportsIndex, x: Material) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> Material:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: typing.SupportsInt | typing.SupportsIndex) -> Material:
        """
        Remove and return the item at index ``i``
        """
class NRT_Impl:
    """
    NRT vacancy calculation method in multi-element targets.
    
    Members:
    
      NRT_element : Ed of the struck atom (default, similar to SRIM).
    
      NRT_average : Average Ed over all target atoms (Crocombette 2019).
    """
    NRT_average: typing.ClassVar[NRT_Impl]  # value = <NRT_Impl.NRT_average: 1>
    NRT_element: typing.ClassVar[NRT_Impl]  # value = <NRT_Impl.NRT_element: 0>
    __members__: typing.ClassVar[dict[str, NRT_Impl]]  # value = {'NRT_element': <NRT_Impl.NRT_element: 0>, 'NRT_average': <NRT_Impl.NRT_average: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class OutputOptions:
    """
    Output file options.  Access via config.Output.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def outfilename(self) -> str:
        """
        Base name for the HDF5 output file (no .h5 extension).
        """
    @outfilename.setter
    def outfilename(self, arg0: str) -> None:
        ...
    @property
    def storage_interval(self) -> int:
        """
        HDF5 write interval [ms].
        """
    @storage_interval.setter
    def storage_interval(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def store_damage_events(self) -> bool:
        """
        Store damage event positions.
        """
    @store_damage_events.setter
    def store_damage_events(self, arg0: bool) -> None:
        ...
    @property
    def store_dedx(self) -> bool:
        """
        Store electronic stopping tables in the output file.
        """
    @store_dedx.setter
    def store_dedx(self, arg0: bool) -> None:
        ...
    @property
    def store_exit_events(self) -> bool:
        """
        Store per-ion exit events.
        """
    @store_exit_events.setter
    def store_exit_events(self, arg0: bool) -> None:
        ...
    @property
    def store_pka_events(self) -> bool:
        """
        Store PKA events.
        """
    @store_pka_events.setter
    def store_pka_events(self, arg0: bool) -> None:
        ...
    @property
    def title(self) -> str:
        """
        Simulation title string.
        """
    @title.setter
    def title(self, arg0: str) -> None:
        ...
class Region:
    """
    Rectangular target region filled with a specific material::
    
        region = opentrim.Region()
        region.id          = "FeLayer"
        region.material_id = "Iron"
        region.origin      = [0.0, 0.0, 0.0]       # nm
        region.size        = [100.0, 100.0, 100.0]  # nm
        config.Target.regions.append(region)
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def id(self) -> str:
        """
        Region identifier.
        """
    @id.setter
    def id(self, arg0: str) -> None:
        ...
    @property
    def material_id(self) -> str:
        """
        ID of the material filling this region.  Must match a Material.id.
        """
    @material_id.setter
    def material_id(self, arg0: str) -> None:
        ...
    @property
    def origin(self) -> list[float]:
        """
        Lower-left corner [x, y, z] in nm.
        """
    @origin.setter
    def origin(self, arg1: list[float]) -> None:
        ...
    @property
    def size(self) -> list[float]:
        """
        Dimensions [Lx, Ly, Lz] in nm.
        """
    @size.setter
    def size(self, arg1: list[float]) -> None:
        ...
class RegionList:
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    @typing.overload
    def __delitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, s: slice) -> RegionList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> Region:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: RegionList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: collections.abc.Iterable) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator[Region]:
        ...
    def __len__(self) -> int:
        ...
    @typing.overload
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: Region) -> None:
        ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: RegionList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: Region) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    @typing.overload
    def extend(self, L: RegionList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: collections.abc.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: typing.SupportsInt | typing.SupportsIndex, x: Region) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> Region:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: typing.SupportsInt | typing.SupportsIndex) -> Region:
        """
        Remove and return the item at index ``i``
        """
class RunOptions:
    """
    Simulation run parameters.  Access via config.Run.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def max_cpu_time(self) -> int:
        """
        Max wall-clock time [s].  0 = unlimited.
        """
    @max_cpu_time.setter
    def max_cpu_time(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def max_no_ions(self) -> int:
        """
        Number of ion histories to simulate.
        """
    @max_no_ions.setter
    def max_no_ions(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def seed(self) -> int:
        """
        RNG seed for reproducibility.
        """
    @seed.setter
    def seed(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def threads(self) -> int:
        """
        Worker threads.  <= 0 = auto (half of hardware_concurrency if >3 cores, else 1).
        """
    @threads.setter
    def threads(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class ScreeningType:
    """
    Nuclear scattering screening potential.
    
    Members:
    
      NoScreening : Unscreened Coulomb.
    
      Bohr
    
      KrC : Krypton-Carbon.
    
      Moliere
    
      ZBL : Ziegler-Biersack-Littmark universal (default).
    
      ZBL_MAGIC : ZBL with MAGIC analytic approximation.
    """
    Bohr: typing.ClassVar[ScreeningType]  # value = <ScreeningType.Bohr: 1>
    KrC: typing.ClassVar[ScreeningType]  # value = <ScreeningType.KrC: 2>
    Moliere: typing.ClassVar[ScreeningType]  # value = <ScreeningType.Moliere: 3>
    NoScreening: typing.ClassVar[ScreeningType]  # value = <ScreeningType.NoScreening: 0>
    ZBL: typing.ClassVar[ScreeningType]  # value = <ScreeningType.ZBL: 4>
    ZBL_MAGIC: typing.ClassVar[ScreeningType]  # value = <ScreeningType.ZBL_MAGIC: 5>
    __members__: typing.ClassVar[dict[str, ScreeningType]]  # value = {'NoScreening': <ScreeningType.NoScreening: 0>, 'Bohr': <ScreeningType.Bohr: 1>, 'KrC': <ScreeningType.KrC: 2>, 'Moliere': <ScreeningType.Moliere: 3>, 'ZBL': <ScreeningType.ZBL: 4>, 'ZBL_MAGIC': <ScreeningType.ZBL_MAGIC: 5>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class SimulationParams:
    """
    Physics model selection.  Access via config.Simulation.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def electronic_stopping(self) -> Stopping:
        """
        Electronic stopping model (Stopping).
        """
    @electronic_stopping.setter
    def electronic_stopping(self, arg0: Stopping) -> None:
        ...
    @property
    def electronic_straggling(self) -> Straggling:
        """
        Electronic straggling model (Straggling).
        """
    @electronic_straggling.setter
    def electronic_straggling(self, arg0: Straggling) -> None:
        ...
    @property
    def intra_cascade_recombination(self) -> bool:
        """
        Allow intra-cascade Frenkel pair recombination.
        """
    @intra_cascade_recombination.setter
    def intra_cascade_recombination(self, arg0: bool) -> None:
        ...
    @property
    def nrt_calculation(self) -> NRT_Impl:
        """
        NRT vacancy method (NRT_Impl).
        """
    @nrt_calculation.setter
    def nrt_calculation(self, arg0: NRT_Impl) -> None:
        ...
    @property
    def screening_type(self) -> ScreeningType:
        """
        Nuclear scattering potential (ScreeningType).
        """
    @screening_type.setter
    def screening_type(self, arg0: ScreeningType) -> None:
        ...
    @property
    def simulation_type(self) -> SimulationType:
        """
        Simulation type (SimulationType).
        """
    @simulation_type.setter
    def simulation_type(self, arg0: SimulationType) -> None:
        ...
class SimulationType:
    """
    Type of ion transport simulation.
    
    Members:
    
      FullCascade : Full damage cascade -- primary ions and all recoils followed.
    
      IonsOnly : Primary ions only; recoil damage estimated by NRT.
    
      CascadesOnly : Recoil cascades only; no primary ion transport.
    """
    CascadesOnly: typing.ClassVar[SimulationType]  # value = <SimulationType.CascadesOnly: 2>
    FullCascade: typing.ClassVar[SimulationType]  # value = <SimulationType.FullCascade: 0>
    IonsOnly: typing.ClassVar[SimulationType]  # value = <SimulationType.IonsOnly: 1>
    __members__: typing.ClassVar[dict[str, SimulationType]]  # value = {'FullCascade': <SimulationType.FullCascade: 0>, 'IonsOnly': <SimulationType.IonsOnly: 1>, 'CascadesOnly': <SimulationType.CascadesOnly: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class SpatialDistribution:
    """
    Ion beam spatial distribution.  Access via config.IonBeam.spatial_distribution.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def center(self) -> list[float]:
        """
        Mean starting position [x, y, z] in nm.
        """
    @center.setter
    def center(self, arg1: list[float]) -> None:
        ...
    @property
    def fwhm(self) -> float:
        """
        Spatial spread FWHM [nm].
        """
    @fwhm.setter
    def fwhm(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def geometry(self) -> Geometry:
        """
        Source geometry (Geometry).
        """
    @geometry.setter
    def geometry(self, arg0: Geometry) -> None:
        ...
    @property
    def type(self) -> Distribution:
        """
        Spatial distribution type (Distribution).
        """
    @type.setter
    def type(self, arg0: Distribution) -> None:
        ...
class Stopping:
    """
    Electronic stopping model.
    
    Members:
    
      Off : No electronic stopping.
    
      SRIM96
    
      SRIM13 : Default.
    
      DPASS : DPASS parametrization v21.06.
    """
    DPASS: typing.ClassVar[Stopping]  # value = <Stopping.DPASS: 2>
    Off: typing.ClassVar[Stopping]  # value = <Stopping.Off: 3>
    SRIM13: typing.ClassVar[Stopping]  # value = <Stopping.SRIM13: 1>
    SRIM96: typing.ClassVar[Stopping]  # value = <Stopping.SRIM96: 0>
    __members__: typing.ClassVar[dict[str, Stopping]]  # value = {'Off': <Stopping.Off: 3>, 'SRIM96': <Stopping.SRIM96: 0>, 'SRIM13': <Stopping.SRIM13: 1>, 'DPASS': <Stopping.DPASS: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class Straggling:
    """
    Electronic energy straggling model.
    
    Members:
    
      Off : Default.
    
      Bohr
    
      Chu
    
      Yang
    """
    Bohr: typing.ClassVar[Straggling]  # value = <Straggling.Bohr: 0>
    Chu: typing.ClassVar[Straggling]  # value = <Straggling.Chu: 1>
    Off: typing.ClassVar[Straggling]  # value = <Straggling.Off: 3>
    Yang: typing.ClassVar[Straggling]  # value = <Straggling.Yang: 2>
    __members__: typing.ClassVar[dict[str, Straggling]]  # value = {'Off': <Straggling.Off: 3>, 'Bohr': <Straggling.Bohr: 0>, 'Chu': <Straggling.Chu: 1>, 'Yang': <Straggling.Yang: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class TargetParams:
    """
    Target geometry and composition.  Access via config.Target::
    
        config.Target.size       = [100.0, 100.0, 100.0]  # nm
        config.Target.cell_count = [200, 1, 1]
        config.Target.materials.append(mat)
        config.Target.regions.append(region)
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def cell_count(self) -> list[int]:
        """
        Grid cell count [Nx, Ny, Nz].
        """
    @cell_count.setter
    def cell_count(self, arg1: list[int]) -> None:
        ...
    @property
    def materials(self) -> MaterialList:
        """
        List of Material objects (MaterialList).
        """
    @materials.setter
    def materials(self, arg0: MaterialList) -> None:
        ...
    @property
    def origin(self) -> list[float]:
        """
        Simulation box origin [x, y, z] in nm.
        """
    @origin.setter
    def origin(self, arg1: list[float]) -> None:
        ...
    @property
    def periodic_bc(self) -> list[int]:
        """
        Periodic boundary flags [x, y, z].  1=periodic, 0=absorbing.
        """
    @periodic_bc.setter
    def periodic_bc(self, arg1: list[int]) -> None:
        ...
    @property
    def regions(self) -> RegionList:
        """
        List of Region objects (RegionList).
        """
    @regions.setter
    def regions(self, arg0: RegionList) -> None:
        ...
    @property
    def size(self) -> list[float]:
        """
        Simulation box dimensions [Lx, Ly, Lz] in nm.
        """
    @size.setter
    def size(self, arg1: list[float]) -> None:
        ...
class TransportParams:
    """
    Ion transport algorithm options.  Access via config.Transport.
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def flight_path_const(self) -> float:
        """
        Constant MFP value [atomic radii].  Used when FlightPath.Constant.
        """
    @flight_path_const.setter
    def flight_path_const(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def flight_path_type(self) -> FlightPath:
        """
        Flight path algorithm (FlightPath).
        """
    @flight_path_type.setter
    def flight_path_type(self, arg0: FlightPath) -> None:
        ...
    @property
    def max_rel_eloss(self) -> float:
        """
        Max relative energy loss per step (dE/E).
        """
    @max_rel_eloss.setter
    def max_rel_eloss(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def mfp_range(self) -> list[float]:
        """
        MFP limits [min, max] in atomic radii.
        """
    @mfp_range.setter
    def mfp_range(self, arg1: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def min_energy(self) -> float:
        """
        Stop tracking ion below this energy [eV].
        """
    @min_energy.setter
    def min_energy(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def min_recoil_energy(self) -> float:
        """
        Stop tracking recoil below this energy [eV].
        """
    @min_recoil_energy.setter
    def min_recoil_energy(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def min_scattering_angle(self) -> float:
        """
        Skip collisions below this angle [degrees].
        """
    @min_scattering_angle.setter
    def min_scattering_angle(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class UserTally:
    """
    User-defined tally configuration::
    
        tally = opentrim.UserTally()
        tally.id    = "depth_profile"
        tally.event = opentrim.Event.Vacancy
        tally.bins.x = list(range(0, 101))
        config.UserTally.append(tally)
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def bins(self) -> UserTallyBins:
        """
        Bin edge definitions (UserTallyBins).
        """
    @bins.setter
    def bins(self, arg0: UserTallyBins) -> None:
        ...
    @property
    def coordinate_system(self) -> CoordSys:
        """
        Local coordinate system for the tally bins (CoordSys).
        """
    @coordinate_system.setter
    def coordinate_system(self, arg0: CoordSys) -> None:
        ...
    @property
    def description(self) -> str:
        """
        Optional short description.
        """
    @description.setter
    def description(self, arg0: str) -> None:
        ...
    @property
    def event(self) -> Event:
        """
        Event that triggers a score (opentrim.Event).
        """
    @event.setter
    def event(self, arg0: Event) -> None:
        ...
    @property
    def id(self) -> str:
        """
        Unique tally identifier.
        """
    @id.setter
    def id(self, arg0: str) -> None:
        ...
class UserTallyBins:
    """
    Bin edge vectors for a user-defined tally.
    
    Use assignment to set bin edges -- do not use .append() on individual fields
    because bin edge fields are plain std::vector<float> returned by value::
    
        tally.bins.x = list(range(0, 101))   # correct -- assignment persists
        tally.bins.E = [0, 1e3, 1e4, 1e5]   # correct
    
        tally.bins.x.append(101)             # wrong -- mutates a copy, silently lost
    """
    def __getitem__(self, arg0: str) -> typing.Any:
        ...
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: str, arg1: typing.Any) -> None:
        ...
    @property
    def E(self) -> list[float]:
        """
        Energy bin edges [eV].
        """
    @E.setter
    def E(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def Tdam(self) -> list[float]:
        """
        PKA damage energy bin edges [eV].
        """
    @Tdam.setter
    def Tdam(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def V(self) -> list[float]:
        """
        Vacancy count bin edges.
        """
    @V.setter
    def V(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def atom_id(self) -> list[float]:
        """
        Atom species id bin edges.
        """
    @atom_id.setter
    def atom_id(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def cosTheta(self) -> list[float]:
        """
        cos(theta) bin edges.
        """
    @cosTheta.setter
    def cosTheta(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def nx(self) -> list[float]:
        """
        Direction cosine nx bin edges.
        """
    @nx.setter
    def nx(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def ny(self) -> list[float]:
        """
        Direction cosine ny bin edges.
        """
    @ny.setter
    def ny(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def nz(self) -> list[float]:
        """
        Direction cosine nz bin edges.
        """
    @nz.setter
    def nz(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def r(self) -> list[float]:
        """
        Radial bin edges [nm].
        """
    @r.setter
    def r(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def recoil_id(self) -> list[float]:
        """
        Recoil generation id bin edges.
        """
    @recoil_id.setter
    def recoil_id(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def rho(self) -> list[float]:
        """
        Cylindrical rho bin edges [nm].
        """
    @rho.setter
    def rho(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def x(self) -> list[float]:
        """
        Bin edges along x [nm].
        """
    @x.setter
    def x(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def y(self) -> list[float]:
        """
        Bin edges along y [nm].
        """
    @y.setter
    def y(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
    @property
    def z(self) -> list[float]:
        """
        Bin edges along z [nm].
        """
    @z.setter
    def z(self, arg0: collections.abc.Sequence[typing.SupportsFloat | typing.SupportsIndex]) -> None:
        ...
class UserTallyList:
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    @typing.overload
    def __delitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, s: slice) -> UserTallyList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> UserTally:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: UserTallyList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: collections.abc.Iterable) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator[UserTally]:
        ...
    def __len__(self) -> int:
        ...
    @typing.overload
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: UserTally) -> None:
        ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: UserTallyList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: UserTally) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    @typing.overload
    def extend(self, L: UserTallyList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: collections.abc.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: typing.SupportsInt | typing.SupportsIndex, x: UserTally) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> UserTally:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: typing.SupportsInt | typing.SupportsIndex) -> UserTally:
        """
        Remove and return the item at index ``i``
        """
__version__: str = '1.1.2'
