# Sphinx configuration for the opentrim Python API documentation.
#
# This is separate from the Doxygen C++ docs (doc/opentrim.dox).  It documents
# only the compiled opentrim extension module by importing it and reading the
# pybind11 docstrings via autodoc.
#
# Build with:
#     pip install -e .[docs] --no-build-isolation
#     sphinx-build -b html doc doc/_build/python

project = "opentrim"
author = "OpenTRIM contributors"

try:
    from opentrim import __version__ as release
except Exception:
    release = ""

# api.rst is the only page, so use it as the root document.
root_doc = "api"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
]

autosummary_generate = True
autodoc_default_options = {
    "members": True,
    "undoc-members": True,
    "show-inheritance": True,
}

html_theme = "alabaster"
exclude_patterns = ["_build", "_build/*"]
