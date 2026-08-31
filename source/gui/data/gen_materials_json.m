## OCTAVE script to generate materials.json file
#
# The script needs to find the following files in the working folder
# 1. - "PeriodicTableJSON.json" from github/Bowserinator. This is used to build the elemental materials cards
# 2. - "Coumpound.dat" from the SRIM-2013/Data folder
#
# It creates a json file with the following structure:
#
# [ {"title": "Category 1", "items": [...]}, {"title": "Category 2", "items": [...]}, ]
#
# where "items" is an array of objects like:
#
#    {
#        "title": "Water  (liquid)",
#        "id": "Water_Liquid (ICRU-276)",
#        "composition_type": "atomic",
#        "density": 1,
#        "Z": [1, 8],
#        "X": [2, 1],
#        "comment": "Chemical Formula:      H ── O ── H",
#        "source": "SRIM-2013"
#    }
#
clear

# Load elements
elements = jsondecode(fileread("PeriodicTableJSON.json")).elements;

group = struct();
group(1).title = "ELEMENTS";
group(1).items = struct();

for i = 1:92,
  m = struct();
  m.id = elements(i).symbol;
  m.title = elements(i).name;
  m.density = elements(i).density;
  m.comment = sprintf("Phase: %s\nCategory: %s", elements(i).phase, elements(i).category);
  m.source = "<a href=\"https://github.com/Bowserinator/Periodic-Table-JSON\" target=\"_blank\">github.com/Bowserinator/Periodic-Table-JSON</a>";
  Z = zeros(1, 1);
  X = ones(1, 1);
  Z(1) = elements(i).number;
  m.Z = Z;
  m.X = X;
  m.composition_type = "atomic";
  group(1).items(i) = m;
end

# Load SRIM compounds from Compound.dat file
fid = fopen("Compound.dat", "r");

if fid == -1
  error("Could not open Compound.dat");
end

line_num = 0;

# load 1 compound
function [compound, line, line_num] = load_compound(fid, line, line_num)

  compound = struct();
  compound.title = strtrim(line(2:37));

  line = fgetl(fid);
  line_num++;

  data = eval(["{" line "}"]);

  if data{1}(1) == "%",
    compound.id = data{1}(2:end);
    compound.composition_type = "mass";
  else
    compound.id = data{1};
    compound.composition_type = "atomic";
  end

  compound.density = data{2};
  n = data{3};
  compound.Z = zeros(1, n);
  compound.X = zeros(1, n);

  for i = 1:n,
    compound.Z(i) = data{4 + (i - 1) * 2};
    compound.X(i) = data{4 + (i - 1) * 2 + 1};
  end

  line = fgetl(fid);
  line_num++;
  compound.bond_struct = eval(["[" line "]"]);

  compound.comment = [];
  comment = {};

  do
    line = fgetl(fid);
    line_num++;

    if line(1) == '$',
      s = line(2:end);

      if length(s) > 0,
        su = native2unicode(uint8(s), "CP850");
        comment{end + 1} = su;
      end

    end

  until ~ischar(line) || line(1) != '$';

  compound.comment = strjoin(comment, "\n");
  compound.source = "SRIM-2013 (<a href=\"http://www.srim.org\" target=\"_blank\">www.srim.org</a>)";

endfunction

# load one group of compounds
function [group, line, line_num] = load_group(fid, line, line_num)
  group = [];
  # skip 1st line
  line = fgetl(fid);
  line_num++;

  if ~ischar(line)
    return
  end

  # get title
  if line(1) != '!'
    error("Line %d: Expected group title, found: %s", line_num, line);
  end

  line = line(2:end); # remove leading '!'
  i = find(line == line(2));
  line(i) = [];
  line = strtrim(line);
  group.title = strjoin(strsplit(line));

  # skip rest of group title
  do
    line = fgetl(fid);
    line_num++;
  until ~ischar(line) || line(1) != '!';

  # start compound
  if line(1) != '*'
    error("Line %d: Expected compound title, found: %s", line_num, line);
  end

  compounds = struct();
  k = 1;

  do
    [compound, line, line_num] = load_compound(fid, line, line_num);
    compounds(k) = compound;
    k++;
  until ~ischar(line) || line(1) != '*';

  group.items = compounds;

endfunction

# skip comment lines
do
  line = fgetl(fid);
  line_num++;
until ~ischar(line) || line(1) != '#';

if line(1) != '!'
  error("Line %d: Expected group start, found: %s", line_num, line);
end

k = 2;

do
  [g, line, line_num] = load_group(fid, line, line_num);

  if !isempty(g)
    group(k) = g;
    k++;
  endif

until ~ischar(line) || line(1) != '!';

fclose(fid);

# Print materials database in json format
fid = fopen('materials.json', 'w');
fwrite(fid, jsonencode(group));
fclose(fid);
