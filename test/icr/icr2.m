clear

# Load HDF5 Results
pkg load hdf5oct

function [nv,nr,nrc]=makehist(Ed,bins,D)
  Td = D(5,:);
  X = D([6 9 10],:); % [V ICR Corr-ICR]

  j = find(Td < Ed);
  Td(j)=[];
  X(:,j) = [];

  nv = zeros(size(bins));
  nr = zeros(size(bins));
  nrc = zeros(size(bins));

  for i=1:length(bins),
    j = find(Td <= bins(i));
    x = mean(X(:,j),2);
    nv(i) = x(1);
    nr(i) = x(2);
    nrc(i) = x(3);

    Td(j) = [];
    X(:,j)=[];
  end

end

E = logspace(1,6,51);

Ed=40;
D = h5load('outEd40eV.h5','/events/pka/event_data');
[V, R, Rc]=makehist(Ed,E,D);
D = h5load('outEd40eVNoCorr.h5','/events/pka/event_data');
[V2, R2, Rc2]=makehist(Ed,E,D);


nrt = E/Ed/2.5;
j=find(E<Ed*2.5);
nrt(j)=1;
j=find(E<Ed);
nrt(j)=1e-6;

xi = nrt;
j=find(E>Ed*2.5);
c = 0.286;
b = -0.568;
xi(j) = (1-c)*nrt(j).^b + c;

figure 1
clf

D = V+R;
loglog(E,D,'.-', ...
  E,V,'.-',...
  E,V+Rc,'.-',...
  E,V2,'.-',...
  E,nrt,E,nrt.*xi)
ylim([0.1 1e4])

xlabel('$T_d$ (eV)','interpreter','latex')
ylabel('$N_d$','interpreter','latex')

h = legend('no ICR, $E_d=40$ eV', ...
  'ICR', ...
  'ICR - excluding Correlated', ...
  'ICR - Correlated=Off', ...
  'NRT', 'arc-dpa', ...
  'location','northwest');
set(h,'interpreter','latex')

title('2MeV Fe in Fe','interpreter','latex')

text(1e3,1,['$R_c$(nm) = 0.8'],'interpreter','latex')

sz = [4 3]*4;
set(1,'PaperUnits','centimeters')
set(1,'PaperSize',sz)
set(1,'PaperPosition',[0 0 sz(1) sz(2)])

print(1,'-dpng','icr2')

figure 2
clf

semilogx(E,V./D,'.-', ...
  E,(V+Rc)./D,'.-', ...
  E,V2./(V2+R2),'.-', ...
  E,xi,E,1./nrt)
ylim([0 1.1])




