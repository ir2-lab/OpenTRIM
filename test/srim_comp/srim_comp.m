clear

# Load OpenTRIM Results
pkg load hdf5oct

Z = [3, 6, 13, 14, 23, 26, 28, 29, 34, ...
     42, 60, 74, 79, 82, 92];
Symbols = {"Li","C", "Al", "Si", "V", "Fe", "Ni", "Cu", "Se", ...
     "Mo", "Nd", "W", "Au", "Pb", "U"};

function V = read_srim(symbol)
  path = ['./Fe5MeV/' symbol "/qc/SRIM Outputs/"];
  S = srim2mat(path);
  dx = abs(S.x(2)-S.x(1));
  V(1) = (sum(S.Vi)+sum(S.Vr))*dx;
  path = ['./Fe5MeV/' symbol "/fc/SRIM Outputs/"];
  S = srim2mat(path);
  dx = abs(S.x(2)-S.x(1));
  V(2) = sum(S.Vr)*dx;
end

function [V,dV,tr] = read_otrim(i)
    path = ['./Fe5MeV/opentrim/t' num2str(i) '.h5'];
    x = h5read(path,'/tally/totals/data');
    dx = h5read(path,'/tally/totals/data_sem');
    V = x(2,[15 2]);
    dV = dx(2,[15 2]);
    tr = x(2,13)/x(2,12);
end


SRIM = zeros(length(Z),2);
OTRIM = zeros(length(Z),2);
dV = zeros(length(Z),2);
tr = zeros(length(Z),1);

for i=1:length(Z)
  disp(sprintf("%d. %s, Z=%d",i,Symbols{i},Z(i)))
  SRIM(i,:) = read_srim(Symbols{i});
  [OTRIM(i,:),dV(i,:),tr(i)] = read_otrim(i);
end

num2str([Z' SRIM OTRIM],3)

figure 1
clf
Y = (OTRIM./SRIM);
dY = 0.003*Y;
errorbar([Z' Z'],(Y-1)*100,dY*100,'o-')
xlim([0 95])
hold on
plot(xlim,[0 0],'k')
hold off
% ylim([0.95 1.1])
title('Projectile: 5MeV Fe, \Delta = (V_{OpenTRIM} - V_{SRIM}) / V_{SRIM}')
xlabel('Target Z')
ylabel('\Delta (%)')
D = abs((Y-1)*100);

legend(...
  sprintf('Q-C, 0 \\leq |\\Delta| < %.1g%%',max(D(:,1))),...
  sprintf('F-C, %.1g \\leq |\\Delta| < %.1g%%',min(D(:,2)),max(D(:,2))))

print -dpng srim_comp_5MeVFe

figure 2
clf
Y = [SRIM(:,1)./SRIM(:,2) OTRIM(:,1)./OTRIM(:,2)];
dY = 0.005*Y;
errorbar([Z' Z'],Y,dY,'o-')
hold on
plot(Z,tr,'^-')
hold off
grid
title('Q-C \div F-C')
legend('SRIM - V','OpenTRIM - V','T_{dam}')

