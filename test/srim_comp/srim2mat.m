function out = srim2mat(folder)
% function out = read_srim_output(folder)
%
% Parses SRIM output files from folder and returns data in
% structure out
%
% Files parsed: VACANCY.txt, IONIZ.txt, PHONON.txt, E2RECOIL.txt,
%               RANGE.txt, NOVAC.txt
%
% If a file does not exist the corresponding data is not created.
% Parsed and failed files are returned in cell arrays out.parsedFiles &
% out.failedFiles.
%
% out has the following data fields (each one is a 100-element vector):
%   x   :  target depth (Angstroem)
%   Vi  :  vacancies from ions (Atoms/ion-Angstroem)
%   Vr  :  vacancies from recoils (Atoms/ion-Angstroem)
%   RC  :  replacement collisions (Atoms/ion-Angstroem)
%   EIi :  ionization energy by ions (eV/ion-Angstroem)
%   EIr :  ionization energy by target recoils (eV/ion-Angstroem)
%   EPi :  phonon energy by ions (eV/ion-Angstroem)
%   EPr :  phonon energy by target recoils (eV/ion-Angstroem)
%   ERi :  recoil energy from ions (eV/ion-Angstroem)
%   ERr :  recoil energy absorbed by target recoils (eV/ion-Angstroem)
%   Ri  :  range distribution of ions (atoms/cm3) / (atoms/cm2)
%   Rr  :  range distribution of recoils (atoms/cm3) / (atoms/cm2)
%
%   Dis:   displacement 3d Displacements/(Angstrom-Ion)
%   EI3:   ionization energy eV/(Angstrom-Ion)
%   EP3:   phonon energy eV /(Angstrom-Ion)
%   Ri3:   rande distribution ions
%   Rr3:   range distribution recoils
%   RC3:   replacement collision Replacements/(Angstrom-Ion)
%   V3r:
%
%

  % Check input
  if ~isfolder(folder)
    error('%s is not a folder');
  endif

  out = struct();
  out.parsedFiles = {};
  out.failedFiles = {};

  fname = file_in_path(folder,'VACANCY.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.Vi = A(:,2);
      out.Vr = A(:,3:end);
      out.parsedFiles = {out.parsedFiles{:}, 'VACANCY.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'VACANCY.txt'};
    end
  end

  fname = file_in_path(folder,'Vacancy-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.V3r = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Vacancy-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Vacancy-3D.txt'};
    end
  end

  fname = file_in_path(folder,'IONIZ.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.EIi = A(:,2); % ioniz. energy by ions
      out.EIr = A(:,3:end); % ioniz. energy by recoils
      out.parsedFiles = {out.parsedFiles{:}, 'IONIZ.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'IONIZ.txt'};
    end
  end

  fname = file_in_path(folder,'Ioniz-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.EI3 = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Ioniz-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Ioniz-3D.txt'};
    end
  end

  fname = file_in_path(folder,'PHONON.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.EPi = A(:,2); % phonon energy by ions
      out.EPr = A(:,3:end); % phonon energy by recoils
      out.parsedFiles = {out.parsedFiles{:}, 'PHONON.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'PHONON.txt'};
    end
  end

  fname = file_in_path(folder,'Phonon-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.EP3 = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Phonon-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Phonon-3D.txt'};
    end
  end

  fname = file_in_path(folder,'E2RECOIL.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.ERi = A(:,2); % recoil energy from ions
      out.ERr = A(:,3:end); % energy absorbed by recoils
      out.parsedFiles = {out.parsedFiles{:}, 'E2RECOIL.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'E2RECOIL.txt'};
    end
  end

  fname = file_in_path(folder,'Displace-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.Dis = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Displace-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Displace-3D.txt'};
    end
  end

  fname = file_in_path(folder,'RANGE.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.Ri = A(:,2); % range of ions
      out.Rr = A(:,3:end); % range of recoils
      out.parsedFiles = {out.parsedFiles{:}, 'RANGE.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'RANGE.txt'};
    end
  end

  fname = file_in_path(folder,'Range-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.Ri3 = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Range-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Range-3D.txt'};
    end
  end

  fname = file_in_path(folder,'NOVAC.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.RC = A(:,2); % Replacement collisions
      out.parsedFiles = {out.parsedFiles{:}, 'NOVAC.txt'};
     else
      out.failedFiles = {out.failedFiles{:}, 'NOVAC.txt'};
    end
  end

  fname = file_in_path(folder,'Recoil-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.Rr3 = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Recoil-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Recoil-3D.txt'};
    end
  end

  fname = file_in_path(folder,'Replace-3D.txt');
  if length(fname),
    A = read_array(fname);
    if ~isempty(A)
      out.x = A(:,1);
      out.RC3 = A(:,2:end);
      out.parsedFiles = {out.parsedFiles{:}, 'Replace-3D.txt'};
    else
      out.failedFiles = {out.failedFiles{:}, 'Replace-3D.txt'};
    end
  end

end

function A = read_array(fname,nrow)
% Helper function to extract data tables from srim output files.
% It searches for nrow contigous lines containing ncol numbers and then
% extracts the data to a [nrow x ncol] array

A = [];

% default SRIM nrow = 100
if (nargin<2)
  nrow = 100;
end

k = 0; % line index
k1 = 0; % index of array start
k2 = 0; % index of array end
ncol = 0; % # of columns

% open file and read lines
fid=fopen(fname,'rt');
while (! feof (fid) )
  text_line = fgetl (fid);
  k=k+1;

  % try to read numbers
  s = sscanf(text_line,'%g',inf);
  ls = length(s);

  if ls
    if ls == ncol,
      k2 = k;
    else
      ncol = ls;
      k1 = k;
      k2 = k;
    endif
  else
    ncol = 0;
  endif

  % if we find nrow contigous rows then break
  if k2-k1+1==nrow,
    break
  endif

end

% read the data array if it was found
if k2-k1+1==nrow,
  frewind(fid);
  fskipl (fid, k1-1);
  A=fscanf(fid,'%g',[ncol nrow])';
endif

fclose(fid);

end
