clear

# get benchmark 1 config
cfg = jsondecode(fileread('../opentrim/b1/config.json'));

run_flag = [0 0 0 0 1];

# function to save config and run opentrim
function run_opentrim(cfg)
  fid = fopen ("cfg.json", "w");
  fputs (fid, jsonencode(cfg));
  fclose (fid);
  system('opentrim -f cfg.json');
end

# Common options
cfg.Driver.max_no_ions = 200;
cfg.Driver.threads = 4;
cfg.Simulation.eloss_calculation = "EnergyLossOff";
cfg.Simulation.intra_cascade_recombination = true;
cfg.Target.materials.composition.El = 0.001;
cfg.Target.materials.composition.Rc = 0.80;

## 1.
disp("Run #1  - Ed=40eV")
disp(" ")
Ed = 40;
cfg.Target.materials.composition.Ed = Ed;
cfg.Target.materials.composition.Er = Ed;
cfg.Transport.min_energy = 1;
cfg.Output.outfilename = 'outEd40eV';

i=1;
if run_flag(i), run_opentrim(cfg); end

## 2.
disp(" ")
disp("Run #2 - Ed=10eV")
disp(" ")
Ed = 10;
cfg.Target.materials.composition.Ed = Ed;
cfg.Target.materials.composition.Er = Ed;
cfg.Transport.min_energy = 1;
cfg.Output.outfilename = 'outEd10eV';

i=2;
if run_flag(i), run_opentrim(cfg); end

## 3.
disp(" ")
disp("Run #3 - Ed=40eV / No corr. recombination")
disp(" ")
cfg.Simulation.correlated_recombination = false;

Ed = 40;
cfg.Target.materials.composition.Ed = Ed;
cfg.Target.materials.composition.Er = Ed;
cfg.Transport.min_energy = 1;
cfg.Output.outfilename = 'outEd40eVNoCorr';

i=3;
if run_flag(i), run_opentrim(cfg); end

## 4.
disp(" ")
disp("Run #4 - Ed=40eV / Move recoil to Rc")
disp(" ")
cfg.Simulation.correlated_recombination = true;
cfg.Simulation.move_recoil = true;

Ed = 40;
cfg.Target.materials.composition.Ed = Ed;
cfg.Target.materials.composition.Er = Ed;
cfg.Transport.min_energy = 1;
cfg.Output.outfilename = 'outEd40eVMvR';

i=4;
if run_flag(i), run_opentrim(cfg); end

## 5.
disp(" ")
disp("Run #5 - Ed=40eV / Move recoil to Rc + Subtract Ed")
disp(" ")
cfg.Simulation.correlated_recombination = true;
cfg.Simulation.move_recoil = true;
cfg.Simulation.recoil_sub_ed = true;

Ed = 40;
cfg.Target.materials.composition.Ed = Ed;
cfg.Target.materials.composition.Er = Ed;
cfg.Transport.min_energy = 1;
cfg.Output.outfilename = 'outEd40eVMvRsubEd';

i=5;
if run_flag(i), run_opentrim(cfg); end
