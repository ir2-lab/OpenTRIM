clear
clf

pkg load hdf5oct
pkg load opentrim

thx = linspace(0.2,0,101); # rad, decreasing 
nx = cos(thx); # increasing, for usertally bins
mux = 0.5*(1-nx);
dOmega = abs(diff(mux))*4*pi;

lbls = {'OpenTRIM', 'MW2005-Geant4', 'MW2005-Quasi Analytic', 'MCNP'};


cfg = opentrim.config();
cfg.from_json(fileread('opentrim/HinC.json'));
display(["Creating OpenTRIM simulation with title: ", cfg.Output.title])
cfg.UserTally(1).bins.nx = nx;
driver = opentrim.driver(cfg);
display("Executing ...")
tic
driver.exec()
t = toc;
display(["Finished. Execution time: ", num2str(t), " seconds."])

H = driver.info().get('/user_tally/AngularDistribution/data')./dOmega;

% load MW2005 data
MW = csvread('./MW2005/MW2005-FIG6.csv');

% load mcnp data
MCNP = load('./mcnp/mcnp_data_h.dat');
M_theta = MCNP(:,1);
M_fnal1 = MCNP(:,2);

figure 1
clf

semilogy(thx(2:end),H,'.-')
hold on
semilogy(MW(:,1),MW(:,2),'linewidth',1.5)
semilogy(MW(:,1),MW(:,3),':','linewidth',1.5)
semilogy(M_theta,M_fnal1,'.-')

hold off

xlabel('angle (rad)','interpreter','latex')
ylabel('Prob. Density (sr$^{-1}$)','interpreter','latex')
title(cfg.Output.title)
legend(lbls,'interpreter','latex')

print2png(gcf,[12 9],'msc_HinC')

cfg = opentrim.config();
cfg.from_json(fileread('opentrim/HeinC.json'));
display(["Creating OpenTRIM simulation with title: ", cfg.Output.title])
cfg.UserTally(1).bins.nx = nx;
driver = opentrim.driver(cfg);
display("Executing ...")
tic
driver.exec()
t = toc;
display(["Finished. Execution time: ", num2str(t), " seconds."])

H = driver.info().get('/user_tally/AngularDistribution/data')./dOmega;

% load MW2005 data
MW = csvread('./MW2005/MW2005-FIG5.csv');


% load mcnp data
MCNP = load('./mcnp/mcnp_data_he.dat');
M_theta = MCNP(:,1);
M_fnal1 = MCNP(:,2);

figure 2
clf

semilogy(thx(2:end),H,'.-')
hold on
semilogy(MW(:,1),MW(:,2),'linewidth',1.5)
semilogy(MW(:,1),MW(:,3),':','linewidth',1.5)
semilogy(M_theta,M_fnal1,'.-')

hold off

xlabel('angle (rad)','interpreter','latex')
ylabel('Prob. Density (sr$^{-1}$)','interpreter','latex')
title(cfg.Output.title)
legend(lbls,'interpreter','latex')

print2png(gcf,[12 9],'msc_HeinC')