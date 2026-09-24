function [S, Z, M, E] = get_projectile_data()

  fid = fopen("projectiles.csv");
  fgetl(fid); % read and discard the header line
  C = textscan(fid, "%d %s %d %f %f", "Delimiter", ",");
  fclose(fid);

  S = C{2};
  Z = C{3};
  M = C{4};
  E = C{5}*1e6; % eV
