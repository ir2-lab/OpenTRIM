function print2png(h,sz,fname)
% function print2eps(h,sz,fname)
%
% Export figure to EPS file
%
%   h  : figure handle (use gcf for current figure)
%   sz : 2 element vector [width height] in centimeters
%   fname : name of the file (.eps extension added automatically)
set(h,'PaperUnits','centimeters')
set(h,'PaperSize',sz)
set(h,'PaperPosition',[1 1 sz(1) sz(2)])

print(h,'-dpng','-r600','-svgconvert',fname)
