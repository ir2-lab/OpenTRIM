#### Fe Cascade recombination - Case 3 ######
#
# 
#
clear

# Load HDF5 Results
pkg load opentrim

# parameters
a0 = 0.28665; % Fe lattice const [nm]
L = 10000; # simulation box size [nm]
# damage model parameters
Ed = [40 40 10]; # 
Ec = [10 10 10]; # cutoff energy
Rc = [0.01 2.8 2.8]*a0; # I-V recombination radius [nm]

# PKA energy
E = [50 logspace(2,5,10)];

# ion histories to run
Nh = 100;

# Create a basic Fe config
cfg = opentrim.config();
cfg.Simulation.simulation_type = 'CascadesOnly';
cfg.Simulation.electronic_stopping = 'Off';
cfg.Simulation.defect_recombination = true;

cfg.Transport.flight_path_type = 'Variable';

cfg.IonBeam.ion.symbol = 'Fe';
cfg.IonBeam.ion.atomic_number = 26;
cfg.IonBeam.spatial_distribution.geometry = 'Volume';
cfg.IonBeam.spatial_distribution.center = [1 1 1]*L/2;

cfg.Target.size = [1 1 1]*L;
cfg.Target.cell_count = [1 1 1];
cfg.Target.periodic_bc = [0 0 0];

Fe.element.symbol = 'Fe';
Fe.X = 1;
Fe.Ed = 40;
Fe.El = 0.001;
Fe.Es = 0.001;
Fe.Er = 40;
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
    cfg.Transport.min_energy = Ec(j);
    cfg.IonBeam.energy_distribution.center = E(i);        
    cfg.Target.materials(1).composition(1).Ed = Ed(j);
    cfg.Target.materials(1).composition(1).Er = Ed(j);
    cfg.Target.materials(1).composition(1).Rc = Rc(j);
    D = opentrim.driver(cfg);
    D.exec(); 
    Nv(i,j) = D.info().get('/tally/totals/data')(2,2);
    k++;

  end
end

function [nrt, arcdpa] = dpa_model(Ed,E)

    L = 2*Ed/0.8;

    nrt = E/L;

    i = find(nrt<1);
    if !isempty(i), nrt(i) = 1; end

    i = find(E<Ed);
    if !isempty(i), nrt(i) = 0; end

    arcdpa = nrt;
    i=find(E>L);
    if !isempty(i),
    c = 0.286;
    b = -0.568;
    arcdpa(i) = nrt(i).*((1-c)*nrt(i).^b + c);
    end
endfunction

[nrt, arc]=dpa_model(Ed(1),E);

figure 1
clf
loglog(E/1000,Nv,'o-',E/1000,nrt(:),'k',E/1000,arc(:),'k--')
grid
xlabel('Damage Energy (keV)')
ylabel('# of Frenkel Pairs')
title('Fe - Defect recombination')
lbls = {};
for i=1:length(Ed),
  lbls{i} = sprintf("E_d=%geV, E_c=%geV, R_c=%ga_0 ",Ed(i),Ec(i),Rc(i)/a0);
end
lbls{length(Ed)+1} = 'NRT';
lbls{length(Ed)+2} = 'arc-dpa';
legend(lbls,'location','northwest')

# print in 480x360px, 82dpi
dpi = 82;
sz =480/dpi*2.54*[4 3]/4;
set(1,'PaperUnits','centimeters')
set(1,'PaperSize',sz)
set(1,'PaperPosition',[1 1 sz(1) sz(2)])
print(1,'-dpng',sprintf('-r%d',dpi),'Fe_Case3')