clear

# Benchmark Number
nb = 7;

# Load HDF5 Results
pkg load hdf5oct
data = h5load(['../ions/b' num2str(nb) '/b' num2str(nb) '.h5']);

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
plot(x,data.tally.defects.Implantations(:,1))
title([titlestr ' - Implanted Ions'])
legend(atom_labels{1})
xlabel('x (nm)')
subplot(2,2,2)
plot(x,data.tally.defects.Implantations(:,2:end))
title([titlestr ' - I'])
legend(atom_labels{2:end})
xlabel('x (nm)')
subplot(2,2,3)
plot(x,data.tally.defects.Vacancies)
title([titlestr ' - V'])
legend(atom_labels)
xlabel('x (nm)')
subplot(2,2,4)
plot(x,data.tally.defects.Replacements)
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
plot(x,data.tally.energy_deposition.Phonons)
title([titlestr ' - Phonons'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('eV')
subplot(2,2,3)
plot(x,data.tally.energy_deposition.PKA)
title([titlestr ' - Er'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('eV')
subplot(2,2,4)
plot(x,(data.tally.energy_deposition.PKA)./(data.tally.defects.PKAs))
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

subplot(2,2,1)
plot(x,data.tally.damage.Tdam(:,2:end), ...
  x,sum(data.tally.damage.Tdam(:,2:end),2), ...
  x,data.tally.energy_deposition.Phonons(:,2:end),...
  x,sum(data.tally.energy_deposition.Phonons(:,2:end),2))
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
plot(x,data.tally.damage.Vnrt(:,2:end), ...
  x,sum(data.tally.damage.Vnrt(:,2:end),2), ...
  x,data.tally.defects.Vacancies(:,2:end),...
  x,sum(data.tally.defects.Vacancies(:,2:end),2))
title([titlestr ' - Vac'])
legend(lbls)
xlabel('x (nm)')


lbls = {};
for i=2:length(atom_labels),
  lbls = { lbls{:}, [atom_labels{i} ' Tdam']};
end
lbls = { lbls{:}, 'Total Tdam', 'Total Eph' };
pka = data.tally.defects.PKAs(:,2:end);

subplot(2,2,3)
plot(x,data.tally.damage.Tdam(:,2:end)./pka, ...
  x,sum(data.tally.damage.Tdam(:,2:end),2)./sum(pka,2), ...
  x,sum(data.tally.energy_deposition.Phonons(:,2:end),2)./sum(pka,2))
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
plot(x,data.tally.damage.Vnrt(:,2:end)./pka,...
     x,sum(data.tally.damage.Vnrt(:,2:end),2)./sum(pka,2),...
     x,sum(data.tally.defects.Vacancies(:,2:end),2)./sum(pka,2))
title([titlestr ' - Vac/PKA'])
legend(lbls)
xlabel('x (nm)')

# Figure 4 = Lost ions/energy vs. x
figure 4
clf
subplot(2,2,1)
plot(x,data.tally.ion_stat.collisions)
title([titlestr ' - Collisions'])
legend(atom_labels)
xlabel('x (nm)')
subplot(2,2,2)
plot(x,data.tally.defects.PKAs)
title([titlestr ' - PKAs'])
legend(atom_labels)
xlabel('x (nm)')

subplot(2,2,3)
plot(x,data.tally.ion_stat.flight_path./data.tally.ion_stat.collisions)
title([titlestr ' - mfp'])
legend(atom_labels)
xlabel('x (nm)')
ylabel('nm')
subplot(2,2,4)
plot(x,data.tally.defects.Lost)
title([titlestr ' - Lost ions'])
legend(atom_labels)
xlabel('x (nm)')

# PDF report name
pname = ['../ions/b' num2str(nb) '/b' num2str(nb) '.pdf'];

## TODO: fix this code for v0.3.3
% Get the table in string form.
# TString = sprintf('{\\bf\\fontsize{20}%s}\n\n',titlestr);
# TString = [TString evalc(['table_compare_all(' num2str(nb) ')'])];


# % Get a fixed-width font.
# FixedWidth = get(0,'FixedWidthFontName');

# figure 5
# clf
# set(gcf,'papertype','a4','paperorientation','landscape')
# % Output the table using the annotation command.
# annotation('textbox',[0.3 0 1 1],'string',TString,...
#     'FontName',FixedWidth,'Units','Normalized',...
#     "verticalalignment",'middle',...
#     'fitboxtotext','off','edgecolor','none');

figure 1
set(gcf,'papertype','a4','paperorientation','landscape')
print(gcf,'-dpdf','-fillpage',pname)

for i=2:4
  figure(i)
  set(gcf,'papertype','a4','paperorientation','landscape')
  print(gcf,'-dpdf','-fillpage','-append',pname)
endfor








