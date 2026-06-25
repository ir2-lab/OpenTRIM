"""Tests for the opentrim Python bindings - Feature A (Config, Driver, Info, HDF5).

Run from the repo root after building the extension:

    pip install -e .[test] --no-build-isolation
    pytest test/python -v
"""

import time

import numpy as np
import pytest

import opentrim


def _wait_until_running(sim, timeout=10.0):
    """Block until the run has produced at least one ion, or time out.

    More robust than a fixed sleep: on a slow or oversubscribed machine the
    first ions may take a while to appear.
    """
    deadline = time.time() + timeout
    while time.time() < deadline:
        if sim.ion_count() > 0:
            return
        time.sleep(0.01)
    raise AssertionError("run produced no ions within the timeout")


# --------------------------------------------------------------------------- #
# Config
# --------------------------------------------------------------------------- #
def test_config_default():
    cfg = opentrim.Config()
    assert cfg.Run.max_no_ions == 100
    assert cfg.Run.max_cpu_time == 0
    assert cfg.Run.threads == 1
    assert cfg.Run.seed == 123456789


def test_config_enum_assignment():
    cfg = opentrim.Config()
    cfg.Simulation.electronic_stopping = opentrim.Stopping.SRIM13
    assert cfg.Simulation.electronic_stopping == opentrim.Stopping.SRIM13


def test_config_validate_error(config):
    # an output file name with invalid characters must be rejected
    config.Output.outfilename = "bad name!"
    with pytest.raises(ValueError):
        config.validate()


def test_config_round_trip_json(config):
    cfg2 = opentrim.Config.from_json(config.to_json())
    assert cfg2.IonBeam.ion.symbol == config.IonBeam.ion.symbol
    assert cfg2.Run.max_no_ions == config.Run.max_no_ions
    assert cfg2.Run.threads == config.Run.threads
    assert cfg2.IonBeam.energy_distribution.center == pytest.approx(
        config.IonBeam.energy_distribution.center)
    # the Target sub-tree must survive serialization too
    assert [m.id for m in cfg2.Target.materials] == \
        [m.id for m in config.Target.materials]
    assert [r.id for r in cfg2.Target.regions] == \
        [r.id for r in config.Target.regions]


# --------------------------------------------------------------------------- #
# Driver
# --------------------------------------------------------------------------- #
def test_driver_mode_a(config):
    sim = opentrim.Driver()
    sim.init(config)
    sim.run()           # Mode A - returns immediately
    sim.wait()
    assert not sim.is_running()
    assert sim.ion_count() == config.Run.max_no_ions


def test_driver_wait(config):
    sim = opentrim.Driver()
    sim.init(config)
    sim.run()
    sim.wait()          # blocks until the run thread finishes
    assert sim.ion_count() == config.Run.max_no_ions


def test_driver_abort(big_config):
    sim = opentrim.Driver()
    sim.init(big_config)
    sim.run()
    _wait_until_running(sim)   # ensure the run is under way, then stop it early
    sim.abort()
    sim.wait()
    assert not sim.is_running()
    assert sim.ion_count() < big_config.Run.max_no_ions


def test_driver_ion_count(big_config):
    sim = opentrim.Driver()
    sim.init(big_config)
    sim.run()
    _wait_until_running(sim)
    c1 = sim.ion_count()
    time.sleep(0.1)
    c2 = sim.ion_count()
    sim.abort()
    sim.wait()
    assert c2 >= c1            # monotonic during the run
    assert sim.ion_count() > 0


# --------------------------------------------------------------------------- #
# Info
# --------------------------------------------------------------------------- #
def test_info_grid_shape(info):
    x = info["target"]["grid"]["X"]
    assert x.shape == (201,)          # Nx + 1 edges, Nx = 200


def test_info_tally_shape(info):
    v, dv = info["tally"]["damage_events"]["Vacancies"]
    assert v.shape == (2, 200, 1, 1)  # [natoms, Nx, Ny, Nz]
    assert dv.shape == v.shape


def test_info_tally_normalized(info):
    v, _ = info["tally"]["damage_events"]["Vacancies"]
    assert np.isfinite(v).all()
    assert (v >= 0).all()
    # per-ion values stay small; raw accumulated counts would be much larger
    assert v.max() < 1e3


def test_info_sem_positive(info):
    _, dv = info["tally"]["damage_events"]["Vacancies"]
    assert (dv >= 0).all()


def test_info_repr(info):
    text = repr(info)
    for key in ("run_info", "target", "ion_beam", "tally"):
        assert key in text


# --------------------------------------------------------------------------- #
# HDF5 persistence
# --------------------------------------------------------------------------- #
def test_hdf5_round_trip(finished_sim, tmp_path):
    path = tmp_path / "round_trip.h5"
    finished_sim.save(str(path))
    sim2 = opentrim.Driver()
    sim2.load(str(path))
    assert sim2.ion_count() == finished_sim.ion_count()


def test_hdf5_tally_match(finished_sim, tmp_path):
    path = tmp_path / "tally_match.h5"
    finished_sim.save(str(path))
    sim2 = opentrim.Driver()
    sim2.load(str(path))
    v1, _ = opentrim.Info(finished_sim)["tally"]["damage_events"]["Vacancies"]
    v2, _ = opentrim.Info(sim2)["tally"]["damage_events"]["Vacancies"]
    assert np.allclose(v1, v2)


def test_hdf5_save_before_run_raises(config, tmp_path):
    # saving with zero ions would write NaN tallies - must raise instead
    sim = opentrim.Driver()
    sim.init(config)
    with pytest.raises(RuntimeError, match="nothing to save"):
        sim.save(str(tmp_path / "empty.h5"))


# --------------------------------------------------------------------------- #
# Introspection / API surface
# --------------------------------------------------------------------------- #
def test_config_substruct_reprs(config):
    # every sub-struct must have a readable repr, not the default <object ...>
    cases = [
        (config.Simulation, "SimulationParams"),
        (config.Transport, "TransportParams"),
        (config.IonBeam, "IonBeamParams"),
        (config.IonBeam.energy_distribution, "EnergyDistribution"),
        (config.IonBeam.spatial_distribution, "SpatialDistribution"),
        (config.IonBeam.angular_distribution, "AngularDistribution"),
        (config.Target, "TargetParams"),
        (config.Run, "RunOptions"),
        (config.Output, "OutputOptions"),
        (config.Target.materials[0], "Material"),
        (config.Target.materials[0].composition[0], "Atom"),
        (config.Target.regions[0], "Region"),
    ]
    for obj, prefix in cases:
        assert repr(obj).startswith(prefix), repr(obj)


def test_standalone_reprs():
    assert repr(opentrim.CoordSys()).startswith("CoordSys")
    assert repr(opentrim.UserTallyBins()).startswith("UserTallyBins")
    assert repr(opentrim.UserTally()).startswith("UserTally")
    assert repr(opentrim.Atom()).startswith("Atom")
    assert repr(opentrim.Material()).startswith("Material")
    assert repr(opentrim.Region()).startswith("Region")


def test_config_dict_access(config):
    config["Run"]["threads"] = 8
    assert config.Run.threads == 8
    config["Simulation"]["electronic_stopping"] = opentrim.Stopping.SRIM13
    assert config.Simulation.electronic_stopping == opentrim.Stopping.SRIM13
    with pytest.raises(KeyError):
        config["NoSuchSection"]


def test_config_nested_assignment(config):
    config.IonBeam.energy_distribution.center = 3.0e6
    assert config.IonBeam.energy_distribution.center == pytest.approx(3.0e6)


def test_element_from_symbol_and_z():
    assert opentrim.Element("Fe").symbol == "Fe"
    assert opentrim.Element(26).symbol == "Fe"        # Z = 26 is iron
    assert opentrim.Element("Fe") == opentrim.Element(26)


def test_element_invalid_raises():
    with pytest.raises(ValueError):
        opentrim.Element("Xx")


def test_element_hashable():
    fe1 = opentrim.Element("Fe")
    fe2 = opentrim.Element(26)
    assert fe1 == fe2
    assert fe1 != opentrim.Element("C")    # __ne__ derives from __eq__
    assert hash(fe1) == hash(fe2)          # equal elements must hash equal
    assert len({fe1, fe2, opentrim.Element("C")}) == 2   # usable in a set


def test_info_repr_html(info):
    html = info._repr_html_()
    assert html.startswith("<div")
    for key in ("run_info", "target", "tally"):
        assert key in html


def test_info_keys_and_contains(info):
    keys = info.keys()
    assert "tally" in keys and "target" in keys
    assert "tally" in info
    assert "does_not_exist" not in info


def test_top_level_reprs(config):
    assert repr(config).startswith("Config")
    assert repr(opentrim.Driver()).startswith("Driver")
    assert repr(opentrim.Element("Fe")).startswith("Element")


def test_vector_fields_roundtrip(config):
    config.Target.size = [10.0, 20.0, 30.0]
    assert list(config.Target.size) == [10.0, 20.0, 30.0]
    config.Target.cell_count = [4, 5, 6]
    assert list(config.Target.cell_count) == [4, 5, 6]
    config.IonBeam.spatial_distribution.center = [7.0, 8.0, 9.0]
    assert list(config.IonBeam.spatial_distribution.center) == [7.0, 8.0, 9.0]
    config.Target.regions[0].origin = [1.0, 2.0, 3.0]
    assert list(config.Target.regions[0].origin) == [1.0, 2.0, 3.0]
    with pytest.raises(TypeError):
        config.Target.size = [1.0, 2.0]               # needs exactly 3 components


def test_mfp_range_validation(config):
    config.Transport.mfp_range = [0.1, 100.0]
    assert list(config.Transport.mfp_range) == pytest.approx([0.1, 100.0])
    with pytest.raises(ValueError):
        config.Transport.mfp_range = [1.0]            # needs exactly 2 values


def test_user_tally_end_to_end(config):
    # define a user tally, run, and read it back through Info (data, sem) tuple
    ut = opentrim.UserTally()
    ut.id = "depth"
    ut.description = "vacancy depth profile"
    ut.event = opentrim.Event.Vacancy
    ut.bins.x = list(range(0, 101, 10))
    config.UserTally.append(ut)
    config.validate()

    sim = opentrim.Driver()
    sim.init(config)
    sim.run()
    sim.wait()

    info = opentrim.Info(sim)
    assert "depth" in info["user_tally"].keys()
    data, sem = info["user_tally"]["depth"]["data"]
    assert data.shape == sem.shape
    assert np.isfinite(data).all()
    assert (sem >= 0).all()
