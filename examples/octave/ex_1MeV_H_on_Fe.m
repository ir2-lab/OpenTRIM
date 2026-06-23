clear

pkg load opentrim

cfg = opentrim.config();
cfg.from_json(fileread('../json/1MeV_H_on_Fe.json'));

N = 100000;
cfg.Run.max_no_ions = N;

d = opentrim.driver(cfg);

tic
d.exec()
toc

results = d.info();
x = results.get('/target/grid/X');
x = x(1:end - 1);
[V, dV] = results.get('/tally/damage_events/Vacancies');

V = squeeze(V);
dV = squeeze(dV);

plot(x, V)
