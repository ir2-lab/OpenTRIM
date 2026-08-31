clear

OTRIM_QC = load('OTRIM_QC.dat');
OTRIM_FC = load('OTRIM_FC.dat');
SRIM_QC = load('SRIM_QC.dat');
SRIM_FC = load('SRIM_FC.dat');
ERROR_QC = load('ERROR_QC.dat');
ERROR_FC = load('ERROR_FC.dat');

[Target_Symb, Target_Z, Mt, rhot, nat, Edt, Elt] = get_target_data();
[Projectile_Symb, Projectile_Z, Projectile_M, Projectile_E] = get_projectile_data();

Nt = length(Target_Z);
Np = length(Projectile_Z);

figure 1
clf

ip = 1;
ig = 1;

set(gcf, 'papertype', 'a4')

for i = 1:Np

  subplot(3, 1, ig)

  Y(:,1) = (OTRIM_QC(:, i) ./ SRIM_QC(:,i) - 1) * 100;
  Y(:,2) = (OTRIM_FC(:, i) ./ SRIM_FC(:,i) - 1) * 100;
  dY(:,1) = ERROR_QC(:, i) ./ SRIM_QC(:,i) * 100;
  dY(:,2) = ERROR_FC(:, i) ./ SRIM_FC(:,i) * 100;

  l1 = sprintf('Q-C, |\\Delta| < %.0f%%', max(abs(Y(:, 1))));
  l2 = sprintf('F-C, |\\Delta| < %.0f%%', max(abs(Y([1:7 9:15], 2))));

  hold on

  xlim([0 95])

  d = 0.1; x = [d (95 - d) (95 - d) d];
  patch(x, [-1 -1 1 1] * 10, [1 1 1] * 0.95, 'EdgeColor', 'none');
  patch(x, [-1 -1 1 1] * 5, [1 1 1] * 0.9, 'EdgeColor', 'none');
  patch(x, [-1 -1 1 1] * 2, [1 1 1] * 0.8, 'EdgeColor', 'none');

  h1 = plot(Target_Z, Y(:, 1), ['o-;' l1 ';']);
  h2 = plot(Target_Z, Y(:, 2), ['^-;' l2 ';']);

  for k = 1:Nt
    text(Target_Z(k)+1, Y(k, 2)+0.5, Target_Symb{k}, ...
         'horizontalalignment', 'center', 'verticalalignment', 'bottom', ...
         'fontsize', 7);
  endfor

  plot(xlim, [0 0], 'k')
  #plot(xlim, [1 1]*-5, 'k--')
  #plot(xlim, [1 1] * 5, 'k--')
  hold off
  ylim([-20 20])
  box on
  title(['Projectile: ' num2str(Projectile_E(i)*1e-6) 'MeV ' Projectile_Symb{i} ', \Delta = (V_{OpenTRIM} - V_{SRIM}) / V_{SRIM}'])
  xlabel('Target Z')
  ylabel('\Delta (%)')

  legend([h1, h2], 'fontname', 'courier')

  if (ig == 3) || (i == Np)

    if (ip == 1)
      print -dpdf -fillpage compare_opentrim_srim
    else
      print -dpdf -fillpage -append compare_opentrim_srim
    end

    clf
    ip = ip + 1;
    ig = 1;
  else
    ig = ig + 1;
  end

endfor
