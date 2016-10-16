set terminal png

set yrange [1:2.2]
set output "../gfx/anom-func.png"
plot [0.38:0.78] "hist.dat" smooth bezier ti "Re(n)", 1.15*exp(-120*pi*(x-0.58)**2)+1 ti "Im(n)"
set output "../gfx/norm-func.png"
plot [0.38:0.78] -0.07/(x-0.375)+2.1 smooth bezier ti "Re(n)", 1.15*exp(-120*pi*(x-0.58)**2)+1 ti "Im(n)"
