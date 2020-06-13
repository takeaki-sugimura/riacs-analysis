# In order to execute this code, you need to specify the folder 
# where this code is located as the working directory of R. 
# The following is an example of specifying a working directory. 
# Please refer to the README file for details.
# 
# Example:
# setwd("C:/RIACS/software_analysis_2")
#

source("S5-functions_04s.R")

# Import data

exp_info <- read_exp_info("experiment_info_euglena.csv")
result <- build_IANode_data(exp_info,F)

area_range <- c(500,2800)
x_range <- c(-1,200)
y_range <- c(-1,200)
tdat2 <- removenoise(result,area_range=area_range,x_range=x_range,y_range=y_range)

# Draw plot

library(mgcv)
xlim=c(-0.1, 0.3)
ylim=c(-0.1, 0.3)
gx0 <- -0.1
gx1 <- 0.3
gy0 <- -0.1+0.02
gy1 <- 0.3+0.02

expnum1 = 3
td3 <- tdat2[tdat2$ExpNum==expnum1,]
x3 <- td3$s_result1  # Component 1 (12C paramylon)
y3 <- td3$s_result3  # Component 3 (13C paramylon)
col3 <- list(h=240,c=100,l=20)  # Green
k3 <- kde2d(x3,y3,n=600,lims=c(xlim,ylim))
k3$z <- k3$z*length(x3)
zl <- c(k3$z)
zu <- c(k3$z)
zlim <- c(0,zu)
cl3 <- contourLines(k3,levels=pretty(zlim,12))
par(mar=c(2.5,2.5,0.5,0.5))
plot(-1,0,xlim=xlim,ylim=ylim,xaxs="i",yaxs="i",axes=F,xlab="",ylab="")
polygon(c(gx0,gx1,gx0),c(gy0,gy1,gy1),col=hcl(30,30,100),border=0)
lines(c(gx0,gx1),c(gy0,gy1),col=2)
if(length(cl3)==0){
	px3 = x3; py3 = y3;
}else{
	isin3 <- in.out(cbind(cl3[[1]]$x,cl3[[1]]$y),cbind(x3,y3))
	px3 = x3[!isin3]; py3 = y3[!isin3];
	for(i in 1:length(cl3)){
		polygon(cl3[[i]],border=hcl(col3$h,col3$c,col3$l,0.3),col=hcl(col3$h,col3$c,col3$l,0.1))
	}
}
par(new=T)
plot(px3,py3,cex=0.2,col=hcl(col3$h,col3$c,col3$l,0.9),xlim=xlim,ylim=ylim,xaxs="i",yaxs="i",axes=F,xlab="",ylab="")
box()
axis(1,at=c(-0.1,0,0.1,0.2,0.3,0.4),labels=T)
axis(2,at=c(-0.1,0,0.1,0.2,0.3,0.4),labels=T)


