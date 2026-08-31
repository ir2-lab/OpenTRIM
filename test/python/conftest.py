"""Shared pytest fixtures for the opentrim bindings.

Builds a small, valid He -> Fe simulation and runs it once per session so the
Info and HDF5 tests can share a single result.
"""

import pytest

import opentrim


def make_config(max_ions=200):
    """A complete, valid configuration: 2 MeV He into a 100 nm Fe slab."""
    cfg = opentrim.Config()
    cfg.IonBeam.ion = opentrim.Element("He")
    cfg.IonBeam.energy_distribution.center = 2e6  # eV

    mat = opentrim.Material()
    mat.id = "Fe"
    mat.density = 7.874
    atom = opentrim.Atom()
    atom.element = opentrim.Element("Fe")
    atom.X = 1.0
    atom.Ed = 40.0
    atom.El = 3.0
    atom.Es = 4.28
    atom.Er = 40.0
    atom.Rc = 0.946
    mat.composition.append(atom)
    cfg.Target.materials.append(mat)

    region = opentrim.Region()
    region.id = "slab"
    region.material_id = "Fe"
    region.origin = [0, 0, 0]
    region.size = [100, 100, 100]
    cfg.Target.regions.append(region)

    cfg.Target.cell_count = [200, 1, 1]
    cfg.Target.size = [100.0, 100.0, 100.0]
    cfg.Simulation.electronic_stopping = opentrim.Stopping.SRIM13
    cfg.Simulation.simulation_type = opentrim.SimulationType.FullCascade
    cfg.Output.outfilename = "pytest_sim"
    cfg.Run.max_no_ions = max_ions
    cfg.Run.threads = 2
    cfg.Run.seed = 12345
    return cfg


@pytest.fixture
def config():
    """A fresh, valid configuration - function-scoped, safe to mutate."""
    return make_config()


@pytest.fixture
def big_config():
    """A long run (many ions) so the abort / ion_count tests can observe it
    while it is still in progress."""
    return make_config(max_ions=1_000_000)


@pytest.fixture(scope="session")
def finished_sim():
    """One completed simulation, shared read-only across tests."""
    sim = opentrim.Driver(make_config())
    sim.run()
    sim.wait()
    return sim


@pytest.fixture(scope="session")
def info(finished_sim):
    """Info view over the completed simulation."""
    return opentrim.Info(finished_sim)
