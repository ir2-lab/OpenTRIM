clear

# Load OpenTRIM Results
pkg load hdf5oct

path = ['../srim_comp/Fe5MeV/Cu/fc/SRIM Outputs/'];
Sfc = srim2mat(path);
path = ['../srim_comp/Fe5MeV/Cu/qc/SRIM Outputs/'];
Sqc = srim2mat(path);

x = Sfc.x*0.1;
dx = abs(x(2)-x(1));

path = ['../srim_comp/Fe5MeV/opentrim/t8.h5'];

figure 1
clf
V = squeeze(h5read(path,'/tally/damage_events/Vacancies'))(:,2);
Vnrt = squeeze(h5read(path,'/tally/pka_damage/Vnrt_LSS'))(:,2);
plot(x,(Sqc.Vi+Sqc.Vr)*10,x,Sfc.Vr*10,x,Vnrt/dx,x,V/dx)
title('5MeV Fe -> Cu, Vacancies/nm')
xlabel('depth (nm)')
legend('SRIM-QC','SRIM-FC','OTRIM-QC','OTRIM-FC')

figure 2
clf
pka = squeeze(h5read(path,'/tally/pka_damage/Pka'))(:,2);
plot(x,(Sqc.Vi)*10,x,Sfc.Vi*10,x,pka/dx)
title('5MeV Fe -> Cu, PKA/nm')
xlabel('depth (nm)')
legend('SRIM-QC','SRIM-FC','OTRIM-FC')

figure 3
clf
impl = squeeze(h5read(path,'/tally/damage_events/Implantations'))(:,1);
plot(x,(Sqc.Ri)*1e-7,x,Sfc.Ri*1e-7,x,impl/dx)
xlabel('depth (nm)')
title('5MeV Fe -> Cu, Implanted Fe/nm')
legend('SRIM-QC','SRIM-FC','OTRIM-FC')


