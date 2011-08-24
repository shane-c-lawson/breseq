###
##
## NAME
##
## plot_error_rate.r
##
## AUTHOR
##
## Jeffrey E. Barrick <jeffrey.e.barrick@gmail.com>
##
## LICENSE AND COPYRIGHT
##
## Copyright (c) 2010 Michigan State University
##
## breseq is free software; you can redistribute it and/or modify it under the terms the 
## GNU General Public License as published by the Free Software Foundation; either 
## version 1, or (at your option) any later version.
##
###

## Args should be in_file=/path/to/input/error_rates.tab out_file=/path/to/output


for (e in commandArgs()) {
  ta = strsplit(e,"=",fixed=TRUE)
  if(! is.na(ta[[1]][2])) {
    temp = ta[[1]][2]
 #   temp = as.numeric(temp) #Im only inputting numbers so I added this to recognize scientific notation
    if(substr(ta[[1]][1],nchar(ta[[1]][1]),nchar(ta[[1]][1])) == "I") {
      temp = as.integer(temp)
    }
    if(substr(ta[[1]][1],nchar(ta[[1]][1]),nchar(ta[[1]][1])) == "N") {
      temp = as.numeric(temp)
    }
    assign(ta[[1]][1],temp)
    cat("assigned ",ta[[1]][1]," the value of |",temp,"|\n")
  } else {
    assign(ta[[1]][1],TRUE)
    cat("assigned ",ta[[1]][1]," the value of TRUE\n")
  }
}


## Old table file was quality and columns with error rates...

X<-read.table(in_file, sep="\t", header=T)

## calculate min probability after removing quality column
Y<-X[-1]
log10_min_error_rate = floor(log10(min(Y)));
min_error_rate = 10**floor(log10(min(Y)));

pdf(out_file, height=6, width=9)

A_col = "green";
A_pch = 15;
C_col = "red"; 
C_pch = 15;
T_col = "blue"; 
T_pch = 15;
G_col = "black"; 
G_pch = 15;
._col = "orange"; 
._pch = 15;

new_plot <- function(plot_title)
{
	#bottom, left, top, right
	par(mar=c(5,5,3,3));
	options(warn=-1)
	plot(0, type="n", lty="solid", log="y", ylim=c(min_error_rate, 1), xlim=c(min(X$quality), max(X$quality)), main=plot_title, lwd=1, axes=F, xlab="", ylab="", las=1, cex.lab=1.2, cex.axis=1.2, cex.main=1.2 )
	options(warn=1)
	box()

	#y-axis
	
	my_tick_at = c(0.0000000001, 0.000000001, 0.00000001, 0.0000001, 0.000001, 0.00001, 0.0001, 0.001, 0.01, 0.1, 1)
	my_tick_at = my_tick_at[(11+log10_min_error_rate):11];

	my_tick_labels = c(expression(10^-10), expression(10^-9), expression(10^-8), expression(10^-7), expression(10^-6), expression(10^-5), expression(10^-4), expression(10^-3), expression(10^-2), expression(10^-1), expression(1))
	my_tick_labels = my_tick_labels[(11+log10_min_error_rate):11];

	axis(2, cex.lab=1.2, las=1, cex.axis=1.2, yaxs="i", at = my_tick_at, label = my_tick_labels)
	title(ylab="Error rate", mgp = c(3.5, 1, 0), cex.lab=1.2)
	
	#x-axis
	axis(1, cex.lab=1.2, cex.axis=1.2, xaxs="i", at=c(0,5,10,15,20,25,30,35,40))
	title(xlab="Base quality score", mgp = c(3, 1, 0), cex.lab=1.2)
}

new_plot("Reference Base: A")
lines(X$quality, X$AC, col=C_col)
points(X$quality, X$AC, pch=C_pch, col=C_col)

lines(X$quality, X$AT, col=T_col)
points(X$quality, X$AT, pch=T_pch, col=T_col)

lines(X$quality, X$AG, col=G_col)
points(X$quality, X$AG, pch=G_pch, col=G_col)

lines(X$quality, X$A., col=._col)
points(X$quality, X$A., pch=._pch, col=._col)

## border="black" incompatible with earlier R versions!
legend("topright", cex=1.1, c("C","T","G","-"), title="Observed Base:", horiz=T, fill=c(C_col,T_col,G_col,._col), bty="n")

new_plot("Reference Base: T")
lines(X$quality, X$TA, col=A_col)
points(X$quality, X$TA, pch=A_pch, col=A_col)

lines(X$quality, X$TC, col=C_col)
points(X$quality, X$TC, pch=C_pch, col=C_col)

lines(X$quality, X$TG, col=G_col)
points(X$quality, X$TG, pch=G_pch, col=G_col)

lines(X$quality, X$T., col=._col)
points(X$quality, X$T., pch=._pch, col=._col)

legend("topright", cex=1.1, c("A","C","G","-"), title="Observed Base:", horiz=T, fill=c(A_col,C_col,G_col,._col), bty="n")


new_plot("Reference Base: C")
lines(X$quality, X$CA, col=A_col)
points(X$quality, X$CA, pch=A_pch, col=A_col)

lines(X$quality, X$CT, col=T_col)
points(X$quality, X$CT, pch=T_pch, col=T_col)

lines(X$quality, X$CG, col=G_col)
points(X$quality, X$CG, pch=G_pch, col=G_col)

lines(X$quality, X$C., col=._col)
points(X$quality, X$C., pch=._pch, col=._col)

legend("topright", cex=1.1, c("A","T","G","-"), title="Observed Base:", horiz=T, fill=c(A_col,T_col,G_col,._col), bty="n")


new_plot("Reference Base: G")
lines(X$quality, X$GA, col=A_col)
points(X$quality, X$GA, pch=A_pch, col=A_col)

lines(X$quality, X$GT, col=T_col)
points(X$quality, X$GT, pch=T_pch, col=T_col)

lines(X$quality, X$GC, col=C_col)
points(X$quality, X$GC, pch=C_pch, col=C_col)

lines(X$quality, X$G., col=._col)
points(X$quality, X$G., pch=._pch, col=._col)

legend("topright", cex=1.1, c("A","T","C","-"), title="Observed Base:", horiz=T, fill=c(A_col,T_col,C_col,._col), bty="n")


new_plot("Reference Base: -")
lines(X$quality, X$.A, col=A_col)
points(X$quality, X$.A, pch=A_pch, col=A_col)

lines(X$quality, X$.T, col=T_col)
points(X$quality, X$.T, pch=T_pch, col=T_col)

lines(X$quality, X$.C, col=C_col)
points(X$quality, X$.C, pch=C_pch, col=C_col)

lines(X$quality, X$.G, col=G_col)
points(X$quality, X$.G, pch=G_pch, col=G_col)

legend("topright", cex=1.1, c("A","T","C","G"), title="Observed Base:", horiz=T, fill=c(A_col,T_col,C_col,G_col), bty="n")


dev.off()