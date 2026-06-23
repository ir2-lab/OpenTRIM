# OpenTRIM Python examples

Jupyter notebooks demonstrating the `opentrim` Python API.

## Setup

Build and install the bindings with the Jupyter extra, then launch JupyterLab:

```bash
pip install -e .[jupyter] --no-build-isolation
jupyter lab examples/python/basic_run.ipynb
```

The notebooks require `opentrim` to be importable in the active kernel. If the
extension was built under WSL, run JupyterLab from that same WSL environment
(or select its interpreter as the notebook kernel).

## Notebooks

| Notebook | Shows |
|----------|-------|
| `basic_run.ipynb` | Configure a simulation, run it, read results, plot the vacancy depth profile. |
| `live_plot.ipynb` | Mode B run with a callback that updates the plot during the run. |
| `parametric_study.ipynb` | Sweep the beam energy and compare total damage. |
