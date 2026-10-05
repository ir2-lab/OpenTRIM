#### Fe Cascade recombination - Case 2 ######
#
# Compare to MD & MARLOWE data from Ortiz2020, fig. 9
#
clear

# Load HDF5 Results
pkg load opentrim

# parameters
a0 = 0.28665; % Fe lattice const [nm]
L = 10000; # simulation box size [nm]
# damage model parameters
Ed = 40; 
Ec = 10;
Rc = [2 2.79 4]*a0; # Recombination radius [nm]

# PKA energy
E = [5 10 20 40 80]*1000;

# ion histories to run
Nh = 100;

# Create a basic Fe config
cfg = opentrim.config();
cfg.Simulation.simulation_type = 'CascadesOnly';
cfg.Simulation.electronic_stopping = 'Off';
cfg.Simulation.defect_recombination = true;

cfg.Transport.flight_path_type = 'Variable';
cfg.Transport.min_energy = Ec;

cfg.IonBeam.ion.symbol = 'Fe';
cfg.IonBeam.ion.atomic_number = 26;
cfg.IonBeam.spatial_distribution.geometry = 'Volume';
cfg.IonBeam.spatial_distribution.center = [1 1 1]*L/2;

cfg.Target.size = [1 1 1]*L;
cfg.Target.cell_count = [1 1 1];
cfg.Target.periodic_bc = [0 0 0];

Fe.element.symbol = 'Fe';
Fe.X = 1;
Fe.Ed = Ed;
Fe.El = 0.001;
Fe.Es = 0.001;
Fe.Er = Ed;
Fe.Rc = 1;

iron.id = 'Iron';
iron.density = 7.8658;
iron.composition = Fe;

reg.id = 'A';
reg.material_id = 'Iron';
reg.size = [1 1 1]*L;

cfg.Target.materials(1) = iron;
cfg.Target.regions(1) = reg;
cfg.Run.max_no_ions = Nh;
cfg.validate();

Nv = zeros(length(E),length(Rc));

k = 1; Nk = length(E)*length(Rc);
for i=1:length(E),
  for j=1:length(Rc)

    disp(sprintf("%d/%d. E = %g, R = %g",k,Nk,E(i),Rc(j)))
    cfg.IonBeam.energy_distribution.center = E(i);
    cfg.Target.materials.composition.Rc = Rc(j);
    D = opentrim.driver(cfg);
    D.exec(); 
    Nv(i,j) = D.info().get('/tally/totals/data')(2,2);
    k++;

  end
end

figure 1
clf
plot(E/1000,Nv,'o-')
xlim([0 100])
ylim([0 500])
xlabel('PKA Energy (keV)')
ylabel('# of Frenkel Pairs')
legend('R_c = 2.0a_0', 'R_c = 2.8a_0', 'R_c = 4.0a_0',...
  'location','northwest')
title('OpenTRIM - Cascade damage in Fe')

# print in 480x360px, 82dpi
dpi = 82;
sz =480/dpi*2.54*[4 3]/4;
#sz =480/dpi*2.54*[16 9]/16;
%sz = [16 12];
set(1,'PaperUnits','centimeters')
set(1,'PaperSize',sz)
set(1,'PaperPosition',[1 1 sz(1) sz(2)])
print(1,'-dpng',sprintf('-r%d',dpi),'Fe_Case2')


