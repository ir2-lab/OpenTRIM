function [S, Z, M, rho, nat, Ed, El] = get_target_data()

  fid = fopen("targets.csv");
  fgetl(fid); % read and discard the header line
  C = textscan(fid, "%d %s %d %f %f %f %f %f", "Delimiter", ",");
  fclose(fid);

  S = C{2};
  Z = C{3};
  M = C{4};
  rho = C{5};
  nat = C{6};
  Ed = C{7};
  El = C{8};
