function plot_benchmark(nb, printfname)
#
# function plot_benchmark(nb, printfname)
#
# This function loads simulation results from OpenTRIM
# for benchmark number nb = 1..7 and produces detailed plots
# of all quantities of interest.
# If printfname is given, a file "printfname.pdf" is created with
# all plots

# nb : 1..8, Benchmark Number
if nb<1 || nb>8,
  error('plot_benchmark: invalid benchmark number. nb=1,2,...,8');
end

if nargin<2,
  printfname = '';
end

# Load HDF5 Results
pkg load hdf5oct
data = h5load(['../opentrim/b' num2str(nb) '.h5']);

# Title
titlestr = data.run_info.title;

# x axis = cell centers
x = data.target.grid.cell_xyz(:,1);

# atom labels
atom_labels = data.target.atoms.label;

# Figure 1 = Defects vs. x
figure 1
clf
subplot(2,2,1)
plot(x,data.tally.damage_events.Implantations(:,1))
title([titlestr ' - Implanted Ions'])
legend(atom_labels{1})
xlabel('x (nm)')
subplot(2,2,2)
plot(x,data.tally.damage_events.Implantations(:,2:end))
title([titlestr ' - I'])
legend(atom_labels{2:end})
xlabel('x (nm)')
subplot(2,2,3)
plot(x,data.tally.damage_events.Vacancies)
title([titlestr ' - V'])
legend(atom_labels)
xlabel('x (nm)')
subplot(2,2,4)
plot(x,data.tally.damage_events.Replacements)
title([titlestr ' - R'])
legend(atom_labels)
xlabel('x (nm)')

# Figure 2 = Energy Deposition vs. x
figure 2
clf
subplot(2,2,1)
plot(x,data.tally.energy_deposition.Ionization)
title([titlestr ' - Ionization'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('eV')
subplot(2,2,2)
plot(x,data.tally.energy_deposition.Lattice)
title([titlestr ' - Phonons'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('eV')
subplot(2,2,3)
plot(x,data.tally.pka_damage.Pka_energy)
title([titlestr ' - Er'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('eV')
subplot(2,2,4)
plot(x,(data.tally.pka_damage.Pka_energy)./(data.tally.pka_damage.Pka))
title([titlestr ' - Er/PKA'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('eV')

# Figure 3 = Damage parameters vs. x
figure 3
clf
lbls = {};
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' Tdam']};
end
lbls = { lbls{:}, 'Total Tdam' };
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' Eph']};
end
lbls = { lbls{:}, 'Total Eph' };

Tdam1 = data.tally.energy_deposition.Lattice + ...
        data.tally.energy_deposition.Stored;
subplot(2,2,1)
plot(x,data.tally.pka_damage.Tdam(:,2:end), ...
  x,sum(data.tally.pka_damage.Tdam(:,2:end),2), ...
  x,Tdam1(:,2:end),...
  x,sum(Tdam1(:,2:end),2))
title([titlestr ' - Tdam'])
legend(lbls)
xlabel('x (nm)')
ylabel('eV')


lbls = {};
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' FC-NRT']};
end
lbls = { lbls{:}, 'Total FC-NRT' };
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' FC']};
end
lbls = { lbls{:}, 'Total FC' };

subplot(2,2,2)
plot(x,data.tally.pka_damage.Vnrt(:,2:end), ...
  x,sum(data.tally.pka_damage.Vnrt(:,2:end),2), ...
  x,data.tally.damage_events.Vacancies(:,2:end),...
  x,sum(data.tally.damage_events.Vacancies(:,2:end),2))
title([titlestr ' - Vac'])
legend(lbls)
xlabel('x (nm)')


lbls = {};
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' Tdam']};
end
lbls = { lbls{:}, 'Total Tdam', 'Total Eph' };
pka = data.tally.pka_damage.Pka(:,2:end);

subplot(2,2,3)
plot(x,data.tally.pka_damage.Tdam(:,2:end)./pka, ...
  x,sum(data.tally.pka_damage.Tdam(:,2:end),2)./sum(pka,2), ...
  x,sum(Tdam1(:,2:end),2)./sum(pka,2))
title([titlestr ' - Tdam/PKA'])
legend(lbls)
xlabel('x (nm)')
ylabel('eV')

lbls = {};
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' FC-NRT']};
end
lbls = { lbls{:}, 'Total FC-NRT', 'Total FC' };

subplot(2,2,4)
plot(x,data.tally.pka_damage.Vnrt(:,2:end)./pka,...
     x,sum(data.tally.pka_damage.Vnrt(:,2:end),2)./sum(pka,2),...
     x,sum(data.tally.damage_events.Vacancies(:,2:end),2)./sum(pka,2))
title([titlestr ' - Vac/PKA'])
legend(lbls)
xlabel('x (nm)')

# Figure 4 = Lost ions/energy vs. x
figure 4
clf
subplot(2,2,1)
plot(x,data.tally.ion_stat.Collisions)
title([titlestr ' - Collisions'])
legend(atom_labels)
xlabel('x (nm)')
subplot(2,2,2)
plot(x,data.tally.pka_damage.Pka)
title([titlestr ' - PKAs'])
legend(atom_labels)
xlabel('x (nm)')

subplot(2,2,3)
plot(x,data.tally.ion_stat.Flight_path./data.tally.ion_stat.Collisions)
title([titlestr ' - mfp'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('nm')
subplot(2,2,4)
plot(x,data.tally.ion_stat.Lost)
title([titlestr ' - Lost ions'])
legend(atom_labels)
xlabel('x (nm)')

# Print to PDF
if length(printfname)>0,


pname = [printfname '.pdf'];

figure 1
set(gcf,'papertype','a4','paperorientation','landscape')
print(gcf,'-dpdf','-fillpage',pname)

for i=2:4
  figure(i)
  set(gcf,'papertype','a4','paperorientation','landscape')
  print(gcf,'-dpdf','-fillpage','-append',pname)
endfor

end









