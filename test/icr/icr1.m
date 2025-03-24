clear

# Load HDF5 Results
pkg load hdf5oct

function [nv,nr]=makehist(Ed,bins,D)
  Td = D(5,:)';
  V = D(6,:)';
  Rep = D(7,:)';
  R = D(9,:)';

  td = Td;
  v=V;
  r=R;
  j = find(td < Ed);
  td(j)=[];
  v(j)=[];
  r(j)=[];

  nv = zeros(size(bins));
  nr = zeros(size(bins));

  for i=1:length(bins),
    j = find(td <= bins(i));
    nv(i) = mean(v(j));
    nr(i) = mean(r(j));

    td(j) = [];
    v(j) = [];
    r(j) = [];
  end

end

E = logspace(1,6,51);

D = h5load('outEd10eV.h5','/events/pka/event_data');
[V10,R10]=makehist(10,E,D);
D = h5load('outEd40eV.h5','/events/pka/event_data');
[V40,R40]=makehist(40,E,D);

nrt = E/100;
j=find(E<100);
nrt(j)=1;
j=find(E<40);
nrt(j)=1e-6;

xi = nrt;
j=find(E>100);
c = 0.286;
b = -0.568;
xi(j) = (1-c)*nrt(j).^b + c;

figure 1
clf

loglog(E/40,V40+R40,'.-', ...
  E/40,V40,'.-',...
  E/10,V10,'.-',...
  E/40,nrt,E/40,nrt.*xi)
ylim([0.1 1e4])
xlim([1 1e4])
set(gca,'ticklabelinterpreter','latex')
xlabel('$T_d / E_d$','interpreter','latex')
ylabel('$N_d$','interpreter','latex')

h = legend('OpenTRIM - no ICR', ...
  'OpenTRIM - ICR, $E_d=40$ eV', ...
  'OpenTRIM - ICR, $E_d=10$ eV', ...
  'NRT', 'arc-dpa', ...
  'location','northwest');
set(h,'interpreter','latex')

title('2MeV Fe in Fe','interpreter','latex')

R_c = 0.8;
text(1e3,1,['$R_c$(nm) = ' num2str(R_c) ' '],'interpreter','latex')

sz = [4 3]*4;
set(1,'PaperUnits','centimeters')
set(1,'PaperSize',sz)
set(1,'PaperPosition',[0 0 sz(1) sz(2)])

print(1,'-dpng','icr1')

semilogx(E/40,V40./(V40+R40),'.-', ...
  E/10,V10./(V10+R10),'.-', ...
  E/40,xi,E/40,1./nrt)
ylim([0 1.1])

