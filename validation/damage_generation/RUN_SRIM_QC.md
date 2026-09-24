# How to run a SRIM benchmark in QC mode

Procedure to run a test benchmark with SRIM in QC mode and store the results.

In this example we assume:
- Projectile: H 1MeV = 1000 keV
- Target: Li

## 1. Decide target thickness

- Start SRIM
- Select "Stopping / Range Tables"
- The window "Ion Stopping and Range Tables" opens
- Select Projectile
  - Press the yellow "PT" button next to "Ion" (PT=periodic table)
  - Select the projectile from the periodic table (H)
- Select target
  - Press the 2nd yellow "PT" button
  - Select the target (Li)
- Press "Calculate Table"
- Press OK in the dialog asking for FILENAME (just accept the default)
- The table opens in a window
- Scroll to the energy (1st column, 1MeV) and read the range (4th column, 51.18μm for Li)
- Set thickness equal to about 2 x Range, e.g., for Li thickness should be 100μm
- Close the table window
- Select "Main Menu" in the "Ion Stopping and Range Tables" window
- You' re back at the main SRIM menu

## 2. Run the simulation

- Select "TRIM" Calculation
- The window "TRIM Setup Window" opens
- Select Projectile
  - Press the yellow "PT" button next to "Ion Data" 
  - Select the projectile from the periodic table (H)
  - Set the** energy in keV** in the respective input box (1000keV)
- Select Target
  - Press the 2nd yellow "PT" button below "Input Elements to Layer"
  - Select the target (Li)
  - Adjust "Damage (eV)" values
    - "Disp": take it from `target.csv` file, col. 7 "Ed (eV)"
    - "Latt": take it from `target.csv` file, col. 8 "El (eV)"
  - Adjust Target layer **Width**
    - The width is right from "Layer 1" entry
    - Write the thickness from step 1 into the "Width" box
    - Select proper units from the dropdown list: Ang (Angstrom=0.1nm) / um (μm) / mm / cm / m / km)
- Select Number of Ions
  - Set "AutoSave at Ion": 1000
  - Set "Total Number of Ions": 1001
- Press "Save Input & Run TRIM" (yellow button)
- The SRIM run widow opens
- Wait for the ions to run

## 3. Save the data

- The simulation completed 1001 ions
- A 1st dialog is shown: "End of simulation, Save data?"
  - **Press "Yes"**
- In the next dialog
  - **Press "Store in SRIM directory (default)"** 
- Press all the **6 small "F" buttons** on the left side, below "DISTRIBUTIONS"
- After each "F" **press "OK"** in the dialog with default option
- Close the simulation window
- A dialog opens "TRIM: Save simulation Data?"
  - **Press "Yes"**
- In the next dialog
  - **Press "Store in SRIM directory (default)"** 

## 4. Copy files

- Open the SRIM installation folder in your File Explorer/Manager
- Select the sub-folders **"SRIM Outputs"** and **"SRIM Restore"**
- Press "Copy"
- Navigate to (or open in another window) "opentrim_srim_comp" folder
- Navigate to (or create if necessary) the subfolder [projectile]/[Target]/qc
  - In the example `H1MeV/Li/qc`
- Paste the 2 folders from SRIM

## END

- Go for the next Target from the `target.csv` file