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

exp_info <- read_exp_info("experiment_info_beads-sort.csv")
result <- build_IANode_data(exp_info,F)


# Draw histograms

sortrate_ave <- vector()
binedges <- seq(0,140,l=100) # bin edges
ylim <- c(0,600) # yaxis range
col1 <- rgb(0,0,240/255,0.5) # color for 1st data
col2 <- rgb(0,153/255,240/255,0.5) # color for 2nd data
cols <- cbind(col1, col2)

for (i in c(1:length(exp_info$DataID))) {
  # Sort beads position
  lnum <- c(i)  
  # result
  exp_num <- exp_info$ExpNum[lnum]
  gtitle <- paste(exp_info$DataID[lnum], exp_info$SampleName[lnum], sep=" ")
  td <- result[result$ExpNum==exp_num,]
  dataID <- exp_info$DataID[lnum]
  
  len <- length(td[,1])
  max_time <- td$s_abs_time[len-1]
  td1 <- td
  x <- table((td1$s_result0 > 500)&(td1$s_result0 < 2000))
  table(td1$s_sort == 1)
  
  # event rate
  len <- length(td1[,1])
  td1$time_sec <- (td1$s_abs_time - td1$s_abs_time[1]) / 1000000
  td1$evrate_ma10[51:len] <- 50/(td1$time_sec[51:len] - td1$time_sec[1:(len-50)])
  sorted <- td1[td1$s_sort == 1,]
  if (length(sorted$s_tx)>11) {
    sorted$srate_ma10[11:length(sorted$s_tx)] <- 10/(sorted$time_sec[11:length(sorted$s_tx)]
                                                     - sorted$time_sec[1:(length(sorted$s_tx)-10)])
    sorted$srate_ma10[1:11] <- 0
  } else {
    sorted$srate_ma10 <- 0
  }
  sortrate_ave <- cbind(sortrate_ave, mean(x=sorted$srate_ma10))
  
  if (i==1){
    h1 <- hist(td1$evrate_ma10, breaks=binedges, col=col1,
               mar=par(mar=c(3.5,3.5,2.5,1.5),mgp=c(2,2,0)),main='', xlab="", ylab="",
               yaxs="i", xaxs="i", ylim=ylim, lty="blank")
  } else {
    h2 <- hist(td1$evrate_ma10, breaks=binedges, col=col2,add=T,
               mar=par(mar=c(3.5,3.5,2.5,1.5),mgp=c(2,2,0)),main='', xlab="", ylab="",
               yaxs="i", xaxs="i", ylim=ylim, lty="blank")
    box()
  }
}

h1$mids[which.max(h1$counts)]
h2$mids[which.max(h2$counts)]

