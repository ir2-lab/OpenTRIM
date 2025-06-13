function benchmark_compare(nb)
#
# function benchmark_compare(nb)
#
# This function loads simulation results from OpenTRIM, SRIM & Iradina
# for benchmark number nb = 1..7 and produces a file "bx.pdf", where
# x stands for nb, with data tables and plots for comparison

# nb : 1..7, Benchmark Number
if nb<1 || nb>7,
  error('plot_benchmark: invalid benchmark number. nb=1,2,...,7');
end

# Load OpenTRIM Results
pkg load hdf5oct
fname = ['../opentrim/b' num2str(nb) '.h5'];

# PDF report name
pname = ['b' num2str(nb) '.pdf'];

# Title
titlestr = h5read(fname,'/run_info/title');

# x axis = cell centers
x = h5read(fname,'/target/grid/cell_xyz')(:,1);
nx = length(x);
Lx = h5read(fname,'/target/grid/X');
Lx = Lx(end) - Lx(1);

# atom labels
atom_labels = h5read(fname,'/target/atoms/label');
atom_symbols = h5read(fname,'/target/atoms/symbol');
natoms = length(atom_labels);

# Ion Energy
E0 = h5read(fname,'/ion_beam/E0');


legends = {};
legends = { legends{:}, 'OpenTRIM'};



[titles, shorttitles, ylabels, data] = getOpenTRIMdata(fname, natoms, atom_labels, atom_symbols, E0);
data = getSrimData(['../srim/b' num2str(nb)],natoms,data,E0);
legends = { 'OpenTRIM', 'SRIM13-FC'};

# for H in Fe get also monolayer SRIM
if nb>=6,
  data = getSrimData(['../srim/ml/b' num2str(nb)],natoms,data,E0);
  legends = { legends{:}, 'SRIM13-ML'};
end

data = getIradinaData(nb,natoms,data,E0);
legends = { legends{:}, 'Iradina'};

# Generate the table in string form.
TString = '';
txt = sprintf('{\\bf\\fontsize{20}Benchmark #%d\n%s}\n\n',nb,titlestr);
TString = [TString txt];
txt = sprintf('Ion energy E0 = %g eV\nTarget depth = %g nm\n\n',E0,Lx);
TString = [TString txt];
txt = sprintf('Summary Table\n\n',nb,titlestr);
TString = [TString txt];
ng = length(titles);
ncol = length(legends);
# write col headers
txt = sprintf('%-15s','Quantity');
for i=1:ncol,
  txt = [txt sprintf('  %10s',legends{i})];
endfor
TString = [TString txt sprintf('\n')];

for i=1:ng,
  txt = sprintf('%-15s',shorttitles{i});
  v = sum(data{i},1);
  if i==ng,
    v = 1 - v;
  endif
  for j=1:ncol,
    txt = [txt sprintf('  %10s',num2str(v(j),3))];
  endfor
  TString = [TString txt sprintf('\n')];
endfor

figure 1
clf

set(gcf,'papertype','a4','paperorientation','portrait')

# Output the table using the annotation command.
# Get a fixed-width font.
FixedWidthFontName = get(0,'FixedWidthFontName');
annotation('textbox',[0 0 1 1],'string',TString,...
    'FontName',FixedWidthFontName,'Units','Normalized',...
    'verticalalignment','middle',...
    'horizontalalignment','center',...
    'fitboxtotext','off','edgecolor','none');

print(gcf,'-dpdf','-fillpage',pname)

ii = 1; ng = length(titles);
ip = 1;
ig = 1;

clf

while ng>0,

  subplot(2,1,ig)
  plot(x,data{ii},'.-');
  xlabel('nm');
  ylabel(ylabels{ii});
  title(titles{ii},'interpreter','latex');
  legend(legends)

  ii++; ng--;
  ig++;
  if ig==3 || ng==0, # close page
    print(gcf,'-dpdf','-fillpage','-append',pname)
    ip++;
    ig=1;
    clf
  endif

endwhile

endfunction

function [titles, shorttitles, ylabels, data] = getOpenTRIMdata(fname, natoms, atom_labels, atom_symbols, E0)
  
  titles = {};
  shorttitles = {};
  ylabels = {};
  data = {};
  ii = 1;
  V = h5read(fname,'/tally/damage_events/Vacancies');
  for i=2:natoms,
    titles{ii} = ['Vacancies of ' atom_labels{i}];
    shorttitles{ii} = ['V(' atom_symbols{i} ')'];
    ylabels{ii} = 'Vacancies / ion';
    data{ii} = V(:,i);
    ii++;
  endfor

  if natoms>2,
    titles{ii} = 'Total Vacancies';
    shorttitles{ii} = ['V(tot)'];
    ylabels{ii} = 'Vacancies / ion';
    data{ii} = sum(V(:,2:end),2);
    ii++;
  endif

  titles{ii} = ['Replacements'];
  shorttitles{ii} = ['R(tot)'];
  ylabels{ii} = 'Replacements / ion';
  data{ii} = sum(h5read(fname,'/tally/damage_events/Replacements'),2);
  ii++;

  titles{ii} = ['Implanted ' atom_labels{1}];
  shorttitles{ii} = ['I(' atom_symbols{1} ')'];
  ylabels{ii} = 'Implantations / ion';
  data{ii} = h5read(fname,'/tally/damage_events/Implantations')(:,1) + ...
             h5read(fname,'/tally/damage_events/Replacements')(:,1);
  ii++;

  titles{ii} = ['Ionization fraction $E_I/E_0$ by ' atom_labels{1}];
  shorttitles{ii} = ['EI(' atom_symbols{1} ')/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = h5read(fname,'/tally/energy_deposition/Ionization')(:,1)/E0;
  ii++;

  titles{ii} = ['Ionization fraction $E_I/E_0$ by recoils'];
  shorttitles{ii} = ['EI(r)/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = sum(h5read(fname,'/tally/energy_deposition/Ionization')(:,2:end),2)/E0;
  ii++;

  titles{ii} = ['Total Ionization fraction $E_I/E_0$'];
  shorttitles{ii} = ['EI/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = sum(h5read(fname,'/tally/energy_deposition/Ionization'),2)/E0;
  ii++;

  titles{ii} = ['Phonon energy fraction $E_{Ph}/E_0$ by ' atom_labels{1}];
  shorttitles{ii} = ['EPh(' atom_symbols{1} ')/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = h5read(fname,'/tally/energy_deposition/Lattice')(:,1)/E0;
  ii++;

  titles{ii} = ['Phonon energy fraction $E_{Ph}/E_0$ by recoils'];
  shorttitles{ii} = ['EPh(r)/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = (sum(h5read(fname,'/tally/energy_deposition/Lattice')(:,2:end),2) + ...
             sum(h5read(fname,'/tally/energy_deposition/Stored')(:,2:end),2))/E0;
  ii++;

  titles{ii} = ['Total Phonon energy fraction $E_{Ph}/E_0$'];
  shorttitles{ii} = ['EPh(r)/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = (sum(h5read(fname,'/tally/energy_deposition/Lattice'),2) + ...
             sum(h5read(fname,'/tally/energy_deposition/Stored'),2))/E0;
  ii++;

  titles{ii} = ['Total fractional energy deposition $(E_I + E_{Ph})/E_0$'];
  shorttitles{ii} = ['1-(EI+EPh)/E0'];
  ylabels{ii} = '1 / ion';
  data{ii} = data{ii-1}+data{ii-4};
  ii++;

endfunction


function dataout = getSrimData(folder,natoms,datain,E0)

  S = srim2mat(folder);
  dx = abs(S.x(2) - S.x(1));

  ii = 1;
  # Vacancies
  for i=2:natoms,
    dataout{ii} = [datain{ii} S.Vr(:,i-1)*dx];
    ii++;
  endfor
  if natoms>2,
    dataout{ii} = [datain{ii} sum(S.Vr,2)*dx];
    ii++;
  endif
  # Replacements
  dataout{ii} = [datain{ii} S.RC*dx];
  ii++;
  # Implantations
  dataout{ii} = [datain{ii} S.Ri*dx*1e-8];
  ii++;
  # Ionization by ions
  dataout{ii} = [datain{ii} S.EIi*dx/E0];
  ii++;
  # Ionization by recoils
  dataout{ii} = [datain{ii} S.EIr*dx/E0];
  ii++;
  # Ionization total
  dataout{ii} = [datain{ii} (dataout{ii-1}+dataout{ii-2})];
  ii++;
  # Phonons by ions
  dataout{ii} = [datain{ii} S.EPi*dx/E0];
  ii++;
  # Phonons by recoils
  dataout{ii} = [datain{ii} S.EPr*dx/E0];
  ii++;
  # Phonons total
  dataout{ii} = [datain{ii} (dataout{ii-1}+dataout{ii-2})];
  ii++;
  # Total energy
  dataout{ii} = [datain{ii} ((S.EIi+S.EIr+S.EPi+S.EPr)*dx/E0)];
  ii++;

endfunction

#Load data from iradina
# path = ['../iradina/b' num2str(nb) '/output/'];
# Number of ions run
# N0 = [20000 20 8000 20000 20000 20000 20000];


function dataout = getIradinaData(nb,natoms,datain,E0)

  path = ['../iradina/b' num2str(nb) '/output/'];
  N0 = [20000 20 8000 20000 20000 20000 20000];
  Nions = N0(nb);

  ii = 1;
  # Vacancies
  if (nb>=3) && (nb<=5),
    A = load('-ascii',[path 'b' num2str(nb) '.vac.z92.m238.029.mat0.elem0']);
    VU = A(:,4)./Nions;
    dataout{ii} = [datain{ii} VU];
    ii++;
    A = load('-ascii',[path 'b' num2str(nb) '.vac.z8.m15.999.mat0.elem1']);
    VO = A(:,4)./Nions;
    dataout{ii} = [datain{ii} VO];
    ii++;
    dataout{ii} = [datain{ii} (VO+VU)];
    ii++;
  else
    A = load('-ascii',[path 'b' num2str(nb) '.vac.sum']);
    dataout{ii} = [datain{ii} A(:,4)./Nions];
    ii++;
  endif
  # Replacements
  A = load('-ascii',[path 'b' num2str(nb) '.repl.sum']);
  replmnts = A(:,4)./Nions;
  dataout{ii} = [datain{ii} replmnts];
  ii++;
  # Implantations
  A = load('-ascii',[path 'b' num2str(nb) '.ions.total']);
  implants = A(:,4)./Nions; # - replmnts;
  dataout{ii} = [datain{ii} implants];
  ii++;
  # Ionization by ions
  dataout{ii} = [datain{ii} zeros(size(implants))];
  ii++;
  # Ionization by recoils
  dataout{ii} = [datain{ii} zeros(size(implants))];
  ii++;
  # Ionization total
  A = load('-ascii',[path 'b' num2str(nb) '.energy.electronic']);
  EIt = A(:,4)./Nions/E0;
  dataout{ii} = [datain{ii} EIt];
  ii++;
  # Phonons by ions
  dataout{ii} = [datain{ii} zeros(size(implants))];
  ii++;
  # Phonons by recoils
  dataout{ii} = [datain{ii} zeros(size(implants))];
  ii++;
  # Phonons total
  A = load('-ascii',[path 'b' num2str(nb) '.energy.phonons']);
  EPt = A(:,4)./Nions/E0;
  dataout{ii} = [datain{ii} EPt];
  ii++;
  # Total energy
  dataout{ii} = [datain{ii} EIt+EPt];
  ii++;

endfunction





  
