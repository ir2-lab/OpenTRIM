clear

# Effect of correlated recombination

# Load HDF5 Results
pkg load hdf5oct

Nh = 1000;

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

D = h5load('outEd40eV.h5','/events/pka/event_data');
[V40,R40]=makehist(40,E,D);
D = h5load('outEd40eVMvR.h5','/events/pka/event_data');
[V40mv,R40mv]=makehist(40,E,D);
D = h5load('outEd40eVMvRsubEd.h5','/events/pka/event_data');
[V40mvs,R40mvs]=makehist(40,E,D);

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

loglog(E,V40+R40,'.-', ...
  E,V40,'.-',...
  E,V40mv,'.-',...
  E,V40mvs,'.-',...
  E,nrt,E,nrt.*xi)
ylim([0.1 1e4])
set(gca,'ticklabelinterpreter','latex')
xlabel('$T_d$ (eV)','interpreter','latex')
ylabel('$N_d$','interpreter','latex')

h = legend('OpenTRIM - no ICR, $E_d=40$ eV', ...
  'OpenTRIM - ICR, $E_d=40$ eV', ...
  'OpenTRIM - ICR, Initial I-V distance = $R_c$', ...
  'OpenTRIM - ICR, + subtract Ed', ...
  'NRT, $E_d=40$ eV', 'arc-dpa', ...
  'location','northwest');
set(h,'interpreter','latex')

title('2MeV Fe in Fe','interpreter','latex')

text(1e3,1,['$R_c = 0.8$ nm'],'interpreter','latex')

sz = [4 3]*4;
set(1,'PaperUnits','centimeters')
set(1,'PaperSize',sz)
set(1,'PaperPosition',[0 0 sz(1) sz(2)])

print(1,'-dpng','icr3')




