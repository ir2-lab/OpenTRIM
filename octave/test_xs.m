clear

% The phase space of (e,s) used in iradina
% e : reduced energy
% s : reduced impact parameter
e = logspace(-6,6,13);
s = logspace(-8,2,46);
[E,S] = meshgrid(e,s);

load_opentrim_funcs

% calc scattering angle
Th = screened_coulomb_theta(E,S,'ZBL');

i = find(Th<0);

num2str([E(i) S(i) Th(i)])



% Plot scattering angle
figure 1
clf
semilogy(s,Th,'.-')
xlabel('s')
ylabel('\theta')
title('Scattering angle \theta(\epsilon,s)')

return


% get back S from (E,Th)
S1 = screened_coulomb_ip(E,Th,'Bohr');
figure 2
clf
% 100/e^(1/6) is the limit where impulse approx is used
loglog(e,S1,'.-',e,100./e.^(1/6))
xlabel('\epsilon')
ylabel("s'")
title('Get back s'' = s(\epsilon,\theta)')

figure 3
clf
loglog(e,abs((S1-S)./S),'.-')
xlabel('\epsilon')
ylabel("\delta")
title("Error \delta = (s'- s)/s")

% calc cross-section
XS = screened_coulomb_xs(E,Th,'Bohr');
figure 4
clf
loglog(e,XS,'.-')
xlabel('\epsilon')
title('d\sigma/d\Omega (\epsilon,\theta)')

