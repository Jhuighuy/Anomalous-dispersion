set terminal png
set encoding utf8

set yrange [1:2.2]
set output "../gfx/anom-func.png"
plot [0.38:0.78] "hist.dat" smooth bezier ti "n", "abs.dat" smooth bezier ti '{/Symbol a}'
set yrange [1.45:1.7]
set output "../gfx/norm-func.png"
plot [0.38:0.78] 0.286458*x**2 - 0.469792*x + 1.70216 smooth bezier ti "n"
