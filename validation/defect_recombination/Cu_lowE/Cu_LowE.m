clear
pkg load opentrim

a0 = 0.3615; # Cu lattice const [nm]

L = 2000; # simulation box size [nm]

# Robinson1974 Fig 13
Ed = [25 25 5]; # displacement
Ec = [25  5 5]; # cutoff energy
Rc = [0.01 0.75 2.5]*a0; # I-V recombination radii [nm]

# Damage energy
E = [10:10:60 80:20:200]; # 100 eV to 100 keV, 3 pts per decade

Nh = 1000; # run so many ions

# Set up opentrim config
cfg = opentrim.config();

cfg.Simulation.simulation_type = 'CascadesOnly';
cfg.Simulation.electronic_stopping = 'Off';
cfg.Simulation.defect_recombination = true;
cfg.Simulation.time_ordered_cascades = true;

cfg.Transport.min_energy = Ec(1);
cfg.Transport.flight_path_type = 'Variable';

cfg.IonBeam.ion.symbol = 'Cu';
cfg.IonBeam.ion.atomic_number = 29;
cfg.IonBeam.spatial_distribution.geometry = 'Volume';
cfg.IonBeam.spatial_distribution.center = [1 1 1]*L/2;

cfg.Target.size = [1 1 1]*L;
cfg.Target.cell_count = [1 1 1];
cfg.Target.periodic_bc = [0 0 0];

Cu.element.symbol = 'Cu';
Cu.X = 1;
Cu.Ed = Ed(1);
Cu.El = 0.001;
Cu.Es = 0.001;
Cu.Er = Ed(1);
Cu.Rc = 1;

copper.id = 'Copper';
copper.density = 8.96;
copper.composition(1) = Cu;

reg.id = 'A';
reg.material_id = 'Copper';
reg.size = [1 1 1]*L;

cfg.Target.materials(1) = copper;
cfg.Target.regions(1) = reg;
cfg.Run.max_no_ions = Nh;
cfg.validate();

Nv = zeros(length(Ed),length(E));


for i=1:length(E),

  disp(sprintf('%d/%d. E = %g eV',i,length(E),E(i)))
  cfg.IonBeam.energy_distribution.center = E(i);

  for j=1:length(Ed),
    cfg.Transport.min_energy = Ec(j);
    cfg.Target.materials(1).composition(1).Ed = Ed(j);
    cfg.Target.materials(1).composition(1).Er = Ed(j);
    cfg.Target.materials(1).composition(1).Rc = Rc(j);
    D = opentrim.driver(cfg);
    D.exec(); 
    Nv(j,i) = D.info().get('/tally/totals/data')(2,2);
  end

end


figure 1
clf
plot(E,Nv,'o-')
grid
xlabel('Damage Energy (eV)')
title('Cu - low energy cascades')
lbls = {};
for i=1:length(Ed),
  lbls{i} = sprintf("E_d=%geV, E_c=%geV, R_c=%ga_0 ",Ed(i),Ec(i),Rc(i)/a0);
end
legend(lbls,'location','northwest')

# print in 480x270px, 82dpi
dpi = 82;
sz =480/dpi*2.54*[16 9]/16;
set(1,'PaperUnits','centimeters')
set(1,'PaperSize',sz)
set(1,'PaperPosition',[1 1 sz(1) sz(2)])
print(1,'-dpng',sprintf('-r%d',dpi),'Cu_LowE')

