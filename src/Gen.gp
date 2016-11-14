set terminal png size 480,330
#set terminal png linewidth 1.5
set obj 1 rectangle behind from screen 0,0 to screen 1,1
set obj 1 fillstyle solid 1.0 fillcolor rgbcolor "#fff0f0f0"
set encoding utf8

set xlabel "Длина волны (нм)"
set xrange [0.38:0.78] 
set mxtics 10
set xtics ("380" 0.38, "480" 0.48, "580" 0.58, "680" 0.68, "780" 0.78) 
#set xtics ("380" 0.38,\
              "" 0.39,\
			  "" 0.40,\
			  "" 0.41,\
			  "" 0.42,\
			  "" 0.43,\
			  "" 0.44,\
			  "" 0.45,\
			  "" 0.46,\
			  "" 0.47,\
           "480" 0.48,\
		      "" 0.49,\
			  "" 0.50,\
			  "" 0.51,\
			  "" 0.52,\
			  "" 0.53,\
			  "" 0.54,\
			  "" 0.55,\
			  "" 0.56,\
			  "" 0.57,\
           "580" 0.58,\
		      "" 0.59,\
			  "" 0.60,\
			  "" 0.61,\
			  "" 0.62,\
			  "" 0.63,\
			  "" 0.64,\
			  "" 0.65,\
			  "" 0.66,\
			  "" 0.67,\
		   "680" 0.68,\
		      "" 0.69,\
			  "" 0.70,\
			  "" 0.71,\
			  "" 0.72,\
			  "" 0.73,\
			  "" 0.74,\
			  "" 0.75,\
			  "" 0.76,\
			  "" 0.77,\
           "780" 0.78\
		   ) 

set ylabel "Показатель преломления"
set yrange [1:2.2]
set ytics ("1" 1, "1.3" 1.3, "1.6" 1.6, "1.9" 1.9, "2.2" 2.2) 
#set ytics ("1" 1,    \
            "" 1.15, \
         "1.3" 1.3,  \
		    "" 1.45, \
         "1.6" 1.6,  \
		    "" 1.75, \
		 "1.9" 1.9,  \
		    "" 2.05, \
		 "2.2" 2.2   \
		 ) 

set grid

set output "../gfx/anom-func1.png"
plot [0.38:0.78] "hist.dat" smooth bezier ti "n" lc rgb "#FF0000", "abs.dat" smooth bezier ti '{/Symbol a}' lc rgb "#0000FF"

set yrange [1.5:1.6]
#set ytics ("1.5" 1.5, "1.55" 1.55, "1.6" 1.6) 
set ytics ( "1.5" 1.5,  \
               "" 1.525,\
           "1.55" 1.55, \
		       "" 1.575, \
		    "1.6" 1.6   \
		   ) 

set output "../gfx/norm-func1.png"
plot [0.38:0.78] 0.286458*x**2 - 0.469792*x + 1.70216 smooth bezier ti "n" lc rgb "#FF0000"
