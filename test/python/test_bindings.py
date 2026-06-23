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
