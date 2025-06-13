clear
clf
pkg load hdf5oct

th = linspace(0,0.2,100); # rad
mux = 0.5*(1-cos(th));
dOmega = diff(mux)*4*pi;

lbls = {'OpenTRIM', 'MW2005-Geant4', 'MW2005-Quasi Analytic', 'MCNP'};

fname = '../msc/opentrim/HinC.h5';
A = h5read(fname,'/events/exit/event_data');
j = find(A(5,:)>=440 & A(2,:)==0);
mu = 0.5*(1-A(8,j));
Th = mean(sqrt(4*mu));
h = histc(mu,mux)/size(mu,2);
H = h(1:end-1)./dOmega;

% load MW2005 data
MW = csvread('../msc/MW2005/MW2005-FIG6.csv');


% load mcnp data
MCNP = load('../msc/mcnp/mcnp_data_h.dat');
M_theta = MCNP(:,1);
M_fnal1 = MCNP(:,2);

figure 1
clf

semilogy(th(2:end),H,'.-')
hold on
semilogy(MW(:,1),MW(:,2),'linewidth',1.5)
semilogy(MW(:,1),MW(:,3),':','linewidth',1.5)
semilogy(M_theta,M_fnal1,'.-')

hold off

xlabel('angle (rad)','interpreter','latex')
ylabel('Prob. Density (sr$^{-1}$)','interpreter','latex')
title(h5read(fname,'/run_info/title'))
legend(lbls,'interpreter','latex')

print2png(gcf,[12 9],'msc_HinC')

fname = '../msc/opentrim/HeinC.h5';
A = h5read(fname,'/events/exit/event_data');
j = find(A(5,:)>=440 & A(2,:)==0);
mu = 0.5*(1-A(8,j));
Th = mean(sqrt(4*mu));
h = histc(mu,mux)/size(mu,2);
H = h(1:end-1)./dOmega;

% load MW2005 data
MW = csvread('../msc/MW2005/MW2005-FIG5.csv');


% load mcnp data
MCNP = load('../msc/mcnp/mcnp_data_he.dat');
M_theta = MCNP(:,1);
M_fnal1 = MCNP(:,2);

figure 2
clf

semilogy(th(2:end),H,'.-')
hold on
semilogy(MW(:,1),MW(:,2),'linewidth',1.5)
semilogy(MW(:,1),MW(:,3),':','linewidth',1.5)
semilogy(M_theta,M_fnal1,'.-')

hold off

xlabel('angle (rad)','interpreter','latex')
ylabel('Prob. Density (sr$^{-1}$)','interpreter','latex')
title(h5read(fname,'/run_info/title'))
legend(lbls,'interpreter','latex')

print2png(gcf,[12 9],'msc_HeinC')