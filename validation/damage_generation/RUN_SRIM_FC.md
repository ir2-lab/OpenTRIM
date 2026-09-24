# How to run a SRIM benchmark in FC mode

Procedure to run a test benchmark with SRIM in FC mode and store the results.

In this example we assume:
- Projectile: H 1MeV = 1000 keV
- Target: Li

## 1. Decide target thickness

- If you have done the QC benchmark already, check for a file named `[Projectile] in [Target].txt` in the respective `[Projectile]/[Target]/qc/SRIM Outputs/` folder. If it exists, then this is the table you need and you may skip some of the below steps. If not, follow all the steps in the below procedure.
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

## 2. Run the simulation for 1 ion

- Start SRIM
- Select "TRIM Calculation"
- The "TRIM Setup Window" opens
- In the top right drop-down list, **"Type of TRIM Calculation"**, select the 2nd option **"Detailed calculation with full Damage Cascades"**
- Select Projectile
  - Press the yellow "PT" button next to "Ion Data" 
  - Select the projectile from the periodic table (H)
  - Set the **energy in keV** in the respective input box (1000keV in this example)
- Select Target
  - Press the 2nd yellow "PT" button below "Input Elements to Layer"
  - Select the target (Li)
  - Adjust "Damage (eV)" values
    - "Disp": take it from `target.csv` file, col. 7 "Ed (eV)" (25 for Li)
    - "Latt": take it from `target.csv` file, col. 8 "El (eV)" (1.1 for Li)
  - Adjust Target layer **Width**
    - The width is right from "Layer 1" entry
    - Write the thickness from step 1 into the "Width" box
    - Select proper units from the dropdown list: Ang (Angstrom=0.1nm) / um (μm) / mm / cm / m / km)
- Select Number of Ions
  - Set "Total Number of Ions": 1
- Press "Save Input & Run TRIM" (yellow/green button)
- You *may* get a grey warning dialog titled **"Change DAMAGE ....Collisions?"** (not always - only if layer width is <= 100)
  - **Press "Yes"** to continue
- You *may* get a white warning **"Energetic Light Ions with Full Cascades?"** (only for light projectiles, H & He)
  - **Press "Continue Calculation"**
- The calculation of 1 ion runs
- You get a grey dialog "Calculation Complete (1 ions)"
  - **Press "Yes = Save data"**
- In the next dialog
  - **Press "Store in SRIM directory (default)"**
- Close the simulation window
- A dialog opens "TRIM: Save simulation Data?"
  - **Press "Yes"**
- In the next dialog
  - **Press "Store in SRIM directory (default)"** 

## 3. Fix the "Lowest E" bug

- Open the SRIM installation folder in your File Explorer/Manager
- Open the file `SRIM Restore/TDATA.sav` in a text editor (preferably Notepad++)
- At the line below **"Lowest E,  Ed(min)  (eV)"**
  - Change the number "1" to the respective value of Ed from `target.csv`, col. 7 (e.g. 25 for Li)
- Save the file

## 4. Continue the run
- At the "TRIM Setup Window" press the grey button **"Resume saved TRIM calc."**
- A dialog opens **"Restore OLD TRIM data"**
  - **Press "Restore TRIM data from SRIM directory"**
- A grey dialog opens "Original Data is restored for review"
  - **Press OK**
- At the "TRIM Setup Window" press the magenta button **"Resume saved TRIM"** (*notice the buttons have changed*) 
- The simulation window opens with a gray dialog "Calculation Complete (1 ions)"
  - **Press "No = Increase number of ions"**
- At the next dialog enter the number of ions (1000) and press OK
- The simulation starts

## 5. Save the data

- The simulation completed the 1000 ions
- A 1st dialog is shown: "End of simulation"
  - **Press "Yes = Save data"**
- In the next dialog
  - **Press "Store in SRIM directory (default)"** 
- Press all the **6 small "F" buttons** on the left side, below “DISTRIBUTIONS”
- After each "F" **press "OK"** in the dialog with default option
- Close the simulation window
- A dialog opens "TRIM: Save simulation Data?"
  - **Press "Yes"**
- In the next dialog
  - **Press "Store in SRIM directory (default)"** 
- Close the TRIM Setup window

## 4. Copy files

- Open the SRIM installation folder in your File Explorer/Manager
- Select the sub-folders **"SRIM Outputs"** and **"SRIM Restore"**
- Press "Copy"
- Navigate to (or open in another window) "opentrim_srim_comp" folder
- Navigate to (or create if necessary) the subfolder `[projectile]/[Target]/fc`
  - In the example `H1MeV/Li/fc`
- Paste the 2 folders from SRIM

## END

- Go for the next Target from the `target.csv` file