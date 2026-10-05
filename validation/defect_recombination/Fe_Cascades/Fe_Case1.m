#### Fe Cascade recombination - Case 1 ######
#
# Compare to MD & SRIM data from Nordlund2015
#
clear
pkg load opentrim

a0 = 286.65/1000; % Fe lattice const [nm]

L = 10000; # simulation box size [nm]

# Model data
Ed = 40; # 
Er = 40;
Ec = 10; # cutoff energy
Rc = 2.8*a0; # I-V recombination radius [nm]

Nh = 1000; # run so many ions

E = [50000 78700];
stopping = {'Off', 'SRIM13'};
recomb = [false, true];

# Set up opentrim config
cfg = opentrim.config();

cfg.Simulation.simulation_type = 'CascadesOnly';

cfg.Transport.flight_path_type = 'Variable';
cfg.Transport.min_energy = Ec;

cfg.IonBeam.ion.symbol = 'Fe';
cfg.IonBeam.ion.atomic_number = 26;
cfg.IonBeam.spatial_distribution.geometry = 'Volume';
cfg.IonBeam.spatial_distribution.center = [1 1 1]*L/2;

cfg.Target.size = [1 1 1]*L;
cfg.Target.cell_count = [1 1 1];
cfg.Target.periodic_bc = [0 0 0];

comp.element.symbol = 'Fe';
comp.X = 1;
comp.Ed = Ed;
comp.El = 0.001;
comp.Es = 0.001;
comp.Er = Ed;
comp.Rc = Rc;

iron.id = 'Iron';
iron.density = 7.8658;
iron.composition(1) = comp;

reg.id = 'A';
reg.material_id = 'Iron';
reg.size = [1 1 1]*L;

cfg.Target.materials(1) = iron;
cfg.Target.regions(1) = reg;
cfg.Run.max_no_ions = Nh;
cfg.validate();

Nv = zeros(length(E),length(recomb));
NRT = zeros(length(E),length(recomb));
Tdam = zeros(length(E),length(recomb));

dNv = zeros(length(E),length(recomb));
dNRT = zeros(length(E),length(recomb));
dTdam = zeros(length(E),length(recomb));

for i=1:length(E),

  disp(sprintf('%d/%d. E = %g eV',i,length(E),E(i)))
  cfg.IonBeam.energy_distribution.center = E(i);
  cfg.Simulation.electronic_stopping = stopping{i};

  for j=1:length(recomb),
    cfg.Simulation.defect_recombination = recomb(j);
    D = opentrim.driver(cfg);
    D.exec(); 
    [X, dX] = D.info().get('/tally/totals/data');
    Nv(i,j) = X(2,2);
    NRT(i,j) = X(2,15); 
    Tdam(i,j)= X(2,13) ; 
    dNv(i,j) = dX(2,2);
    dNRT(i,j) = dX(2,15); 
    dTdam(i,j)= dX(2,13) ; 
  end

end

disp(sprintf('NRT-LSS approx 78.7keV              %g +/- %g',NRT(2,1),dNRT(2,1)))
disp(sprintf('Full cascade 50keV + no stopping    %g +/- %g',Nv(1,1),dNv(1,1)))
disp(sprintf('Full cascade 50keV + no stopping + Recomb  %g +/- %g',Nv(1,2),dNv(1,2)))
disp(sprintf('Full cascade 78.7keV                %g +/- %g',Nv(2,1),dNv(2,1)))
disp(sprintf('Full cascade 78.7keV + Recomb       %g +/- %g',Nv(2,2),dNv(2,2)))




