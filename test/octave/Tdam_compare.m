clear

pkg load hdf5oct
pkg load ir2oct

# Compare Tdam/pka calculated by OpenTRIM and by SRIM
# with any of the methods proposed in Lin2023

function TdamS = getSrimTdam(folder,natoms,E0,Eb)

  S = srim2mat(folder);
  dx = abs(S.x(2) - S.x(1));

  pka = sum(S.Vi)*dx;

  % methods
  % D1
  % Tdam = beam energy absorbed by target atoms-target atom energy lost to ionization
  TdamS(1) = (sum(sum(S.ERr(1:end))) - sum(S.EIr))*dx;

  % D2
  % Tdam = Insidend beam energy - beam energy lost to ionization -
  % target atom energy lost to ionization - beam energy lost to phonons
  TdamS(2) = E0 - sum(S.EIi + S.EIr + S.EPi)*dx;

  % D3
  % Tdam = Target atom energy lost to phonons - beam energy lost to phonons
  TdamS(3) = sum(S.EPr - S.EPi)*dx;

  % D1*
  % Tdam = Beam energy absorbed by target atoms - target atom energy lost to
  % to ionization + beam energy lost to phonon production
  TdamS(4) = (sum(sum(S.ERr(:,1:end))) - sum(S.EIr) + sum(S.EPi))*dx; % ?


  % D2* = D3* (according to my understanding)
  % Tdam = beam energy lost to lattice binding energy + target ato energy
  % lost to lattice binding energy (Eb*\nu) + beam energy lost to phonon
  %  production + target atom energy lost to phonons
  TdamS(5) = (Eb*sum(S.Vi) + Eb*sum(sum(S.Vr(:,1:end)))+...
  sum(S.EPi+S.EPr))*dx;


  % D2**
  % Tdam = Incident ion beam energy - beam energy lost to ionization -
  % target atom energy lost to ionization
  TdamS(6) = E0 - sum(S.EIi + S.EIr)*dx;

  % D3**
  % Tdam = Target atom energy lost to lattice binding energy + target atom
  % energy lost to phonons
  TdamS(7) = (Eb*sum(sum(S.Vr(:,1:end)))+  ...
  sum(sum(S.EPr(:,1:end))))*dx;

  % D3***
  % Tdam = beam energy lost to lattice binding energy + target atom energy
  % lost to lattice binding energy + target atom energy lost to phonons
  TdamS(8) = (Eb*(sum(S.Vi) + sum(sum(S.Vr(:,1:end))))+  ...
  sum(sum(S.EPr(:,1:end))))*dx;

  TdamS /= pka;

endfunction

function [titlestr, Tdam, TdamS] = getTdam(nb)

  #Load OpenTRIM Results
  fname = ['../opentrim/b' num2str(nb) '.h5'];

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

  % SRIM binding Energy
  Eb = [3, 3, 3.3, 3.3, 3.3, 3, 3];

  # Tdam
  Tdam = sum(sum(h5read(fname,'/tally/pka_damage/Tdam')(:,2:end),2))/...
  sum(sum(h5read(fname,'/tally/pka_damage/Pka')(:,2:end),2));

  TdamS = getSrimTdam(['../srim/b' num2str(nb)],natoms,E0,Eb(nb));

endfunction

figure 1
clf

methods = {'D1','D2','D3','D1*','D2*|D3*','D2**','D3**','D3***'};

for nb=1:7,
  [titlestr, Tdam, TdamS] = getTdam(nb);
  subplot(4,2,nb)
  plot(TdamS/Tdam,'o',[0 10],[1 1],'k')
  xlim([0.5 8.5])
  ylim([0.855 1.145])
  set(gca,'xtick',1:8)
  set(gca,'xticklabel',methods)
  title([num2str(nb) '. ' titlestr '. T_{dam}=' num2str(Tdam,3) ' eV'])
endfor

set(gcf,'papertype','a4','paperorientation','portrait')

print(gcf,'-dpdf','-fillpage','Tdam_compare')

