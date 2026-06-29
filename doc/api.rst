opentrim Python API
===================

Python bindings for the OpenTRIM Monte Carlo ion transport simulator.
The API has three classes: :class:`~opentrim.Config` to set up a simulation,
:class:`~opentrim.Driver` to run it, and :class:`~opentrim.Info` to read the
results.

.. code-block:: python

    import opentrim

    config = opentrim.Config()
    config.IonBeam.ion = opentrim.Element("He")
    config.IonBeam.energy_distribution.center = 2e6   # eV
    # ... define target, run options ...
    config.validate()

    sim = opentrim.Driver()
    sim.init(config)
    sim.run()
    sim.wait()

    info = opentrim.Info(sim)
    v, dv = info["tally"]["damage_events"]["Vacancies"]

Config
------

.. autoclass:: opentrim.Config
   :members:

Driver
------

.. autoclass:: opentrim.Driver
   :members:

Info
----

.. autoclass:: opentrim.Info
   :members:

Element
-------

.. autoclass:: opentrim.Element
   :members:

Enumerations
------------

.. autoclass:: opentrim.SimulationType
.. autoclass:: opentrim.ScreeningType
.. autoclass:: opentrim.Stopping
.. autoclass:: opentrim.Straggling
.. autoclass:: opentrim.NRT_Impl
.. autoclass:: opentrim.FlightPath
.. autoclass:: opentrim.Distribution
.. autoclass:: opentrim.Geometry
.. autoclass:: opentrim.Event
