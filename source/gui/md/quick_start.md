This is a short guide to running your first **OpenTRIM** simulation. 

The easiest way is to modify an existing example. This is what is described in the following steps.

### 1. Click the "Examples" button. 

![intro](./images/intro.png) 

### 2. Double-click one of the examples. 
We'll use "1MeV Xe on UO2" in this guide

![intro](./images/intro22.png) 
   

### 3. Click OK on the pop-up box 
This will discard the current simulation.

![intro](./images/intro33.png)   

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

![intro](./images/run12.png)

  - After reviewing or modifying the options, press the **play button** (1) at the bottom left to **run the simulation**.
  
  - Change the number of ions to be run (2), the number of threads to use (3), ect in the relevant boxes 
  
  - The simulation progress and other data are depicted in real time.

### 5 "Summary" panel
   
![intro](./images/sum1.png)

This panel summarizes key simulation results in four tables: 
   - Damage Events (I)
   - Energy Deposition (II)
   - PKA damage (III)
   - Ion Statistics (IV)

The data is updated in real time. Statistical variance is also shown.

### 6 "Plots" panel
   
![intro](./images/plots11.png)

In this panel, simulation data are plotted in real time.

Select the tally table to plot from the list on the left side.

Below the graph there are controls for setting the axis (X/Y/Z) and selecting
which ion or target atom to show. 

All plots can also be exported to .pdf or .csv format by pressing **Export** button. 
