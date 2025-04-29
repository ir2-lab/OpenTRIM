# opentrim-gui - Quick start guide {#opentrim_gui}

This is a short guide to running your first **OpenTRIM** simulation using `opentrim-gui`.

The easiest way is to modify an existing example. This is what is described in the following steps.

Run the program, either by entering `opentrim-gui` in the command line or by double-clicking the executable file, and start:

### 1. Click the "Examples" button. 

![Click the "Examples" button](./source/gui/md/images/intro.png) 

### 2. Double-click one of the examples. 
We'll use "3 MeV Xe on UO2" in this guide

![Double-click one of the examples](./source/gui/md/images/intro22.png) 
   

### 3. Click OK on the pop-up box 
This will discard the current simulation.

![Click OK on the pop-up box](./source/gui/md/images/intro33.png)   

### 4. The program switches to the "Config" panel

In this panel there are several tabs with configuration options
  - On top there is the user-defined simulation title
  - **General Simulation options (I)** 
     
    This tab contains options regarding simulation types, physical models and processes. 
    Further details regarding these can be found in [Documentation](https://ir2-lab.gitlab.io/opentrim/) .

  - **Ion transport options (II)**

    This tab contains options like energy cutoff, flight path sampling, etc.

  - **Ion Source (III)**

    Define source particle type, energy, spatial and directional distribution.

  - **Target (IV)**

    Define target size, materials and configuration.

  - **Output (V)**
    
    Set options for output data handling.

  - **JSON (VI)**

    Inspect the configuration data in json format.

![Inspect the configuration](./source/gui/md/images/run12.png)

  - After reviewing or modifying the options, press the **play button** (1) at the bottom left to **run the simulation**.
  
  - Change the number of ions to be run (2), the number of threads to use (3), ect in the relevant boxes 
  
  - The simulation progress and other data are depicted in real time.

### 5. "Summary" panel
   
!["Summary" panel](./source/gui/md/images/sum1.png)

This panel summarizes key simulation results in four tables: 
   - Damage Events (I)
   - Energy Deposition (II)
   - PKA damage (III)
   - Ion Statistics (IV)

The data is updated in real time. Statistical variance is also shown.

### 6. "Plots" panel
   
!["Plots" panel](./source/gui/md/images/plots11.png)

In this panel, simulation data are plotted in real time.

Select the tally table to plot from the list on the left side **(1)**.

Below the graph there are controls for setting the axis (X/Y/Z) and selecting
which ion or target atom to show. 

All plots can also be exported to .pdf or .csv format by pressing **Export** button **(2)**. 

### 7. Save results

In order to save your work, return to the **"Welcome" panel** and click on **"Save"** or **"Save As"** button. 

You have the option to save:
- Only the configuration in a `.json` file (see \ref json_config "JSON details here")
- Both configuration + data in a HDF5 archive file (extention `.h5`, more \ref out_file "details on HDF5 here")


