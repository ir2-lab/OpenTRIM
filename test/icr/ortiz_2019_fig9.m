## Compare to Ortiz 2019 Fig. 9

clear

# Load HDF5 Results
pkg load hdf5oct
# for printing out figure
pkg load ir2oct

# get benchmark 1 config
cfg = jsondecode(fileread('fe.json'));

a0 = 0.28665; % Fe lattice const [nm]

Ed = 40; # Fe
Rc = [2 2.79 4]*a0; # Recombination radius [nm]

E = [5 10 20 40 80]*1000;
V = zeros(length(E),length(Rc));

# Common options
cfg.Run.max_no_ions = 1000;
cfg.Run.threads = 4;
cfg.Target.materials.composition.Ed = Ed;
cfg.Target.materials.composition.Er = Ed;
cfg.Target.materials.composition.El = 3;
cfg.Target.materials.composition.Rc = 0.8;
cfg.Transport.min_energy = 5;

cfg.Output.outfilename = 'fe1';
cfg.Output.store_pka_events = false;
cfg.Output.store_damage_events = false;

cfg.Simulation.intra_cascade_recombination = true;
cfg.Simulation.time_ordered_cascades = true;
cfg.Simulation.move_recoil = false;
cfg.Simulation.recoil_sub_ed = false;


# function to save config and run opentrim
function run_opentrim(cfg)
  fid = fopen ("cfg.json", "w");
  fputs (fid, jsonencode(cfg));
  fclose (fid);
  [a,b] = system('opentrim -f cfg.json');
end

function [nrt, arcdpa] = dmodel(Ed,E)
  L = 2*Ed/0.8;
  nrt = E/L;
  j=find(E<L);
  nrt(j)=1;
  j=find(E<Ed);
  nrt(j)=1e-6;

  xi = nrt;
  j=find(E>100);
  c = 0.286;
  b = -0.568;
  xi(j) = (1-c)*nrt(j).^b + c;

  arcdpa = xi.*nrt;

end


for i=1:length(E),
  for j=1:length(Rc)

    disp(sprintf("E = %g, R = %g",E(i),Rc(j)))
    cfg.IonBeam.energy_distribution.center = E(i);
    cfg.Target.materials.composition.Rc = Rc(j);
    run_opentrim(cfg); 
    d = h5load('fe1.h5','/tally/totals/data');
    V(i,j) = d(2,2);

  end
end

[nrt, adpa] = dmodel(Ed,E);

figure 1
clf
x = E/1000;
plot(x,V,'o-',x,nrt,'k',x,adpa,'k--')
xlim([0 100])
ylim([0 500])
xlabel('PKA Energy (keV)')
ylabel('# of Frenkel Pairs')
legend('R_c = 2.0a_0', 'R_c = 2.8a_0', 'R_c = 4.0a_0',...
'NRT','arc-dpa','location','northwest')
title('OpenTRIM - intra-cascade recombination in Fe')
print2png(gcf,[12 9],'ortiz_2019_fig9')


