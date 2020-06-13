# Analysis functions

removenoise <- function(tdat,area_range,x_range,y_range){
	total <- table(tdat$ExpNum)
	tmp <- table(tdat$ExpNum,(tdat$s_result0>=area_range[1]&tdat$s_result0<area_range[2]))
	if(sum(colnames(tmp)=="TRUE")==1){
		ok <- tmp[,colnames(tmp)=="TRUE"]
	}else{
		ok <- 0
	}
	tb <- cbind(total,ok,fail=total-ok,ratio=ok/total*100)
	cat("area:",as.character(area_range[1]),"-",as.character(area_range[2]),"\n")
	print(tb)
	tmp <- table(tdat$ExpNum,(tdat$s_result0>=area_range[1]&tdat$s_result0<area_range[2]&tdat$s_max_xpos<x_range[2]&tdat$s_min_xpos>x_range[1]&tdat$s_max_ypos<y_range[2]&tdat$s_min_ypos>y_range[1]&tdat$s_max_xpos>tdat$s_min_xpos&tdat$s_max_ypos>tdat$s_min_ypos))
	if(sum(colnames(tmp)=="TRUE")==1){
		ok <- tmp[,colnames(tmp)=="TRUE"]
	}else{
		ok <- 0
	}
	tb <- cbind(total,ok,fail=total-ok,ratio=ok/total*100)
	cat("area:",as.character(area_range[1]),"-",as.character(area_range[2]),"& x:",as.character(x_range[1]),"-",as.character(x_range[2]),"& y:",as.character(y_range[1]),"-",as.character(y_range[2]),"\n")
	print(tb)
	return(tdat[tdat$s_result0>=area_range[1]&tdat$s_result0<area_range[2]&tdat$s_max_xpos<x_range[2]&tdat$s_min_xpos>x_range[1]&tdat$s_max_ypos<y_range[2]&tdat$s_min_ypos>y_range[1]&tdat$s_max_xpos>tdat$s_min_xpos&tdat$s_max_ypos>tdat$s_min_ypos,])
}



# データ読み込み
library(MASS)

read_exp_info <- function(exp_info_file){
	exp_info <- read.csv(exp_info_file)
	exp_info <- exp_info[!is.na(exp_info$Date),]
	return(exp_info)
}

build_IANode_data <- function(exp_info,readLog=F){	
	result <- data.frame()
	curtx <- 0
	for(i in 1:dim(exp_info)[1]){
		anal_srs_file <- as.character(exp_info$SrsImgAres[i])
		tmp <- read_S5_data(anal_srs_file)

		if(readLog){
			log_file <- paste(exp_folder,as.character(exp_info$IANode_log[i]),sep="")
			tmp2 <- read_log(log_file)
			tmp2 <- tmp2[-1,]
			tmp <- cbind(tmp,tmp2)
		}
		tmp$Date <- exp_info$Date[i]
		tmp$ExpSeries <- exp_info$ExpSeries[i]
		tmp$ExpNum <- exp_info$ExpNum[i]
		tmp$SampleName <- exp_info$SampleName[i]
		tmp$Sample <- paste(exp_info$ExpNum[i],exp_info$SampleName[i],sep="_")
		tmp$DataID <- exp_info$DataID[i]
		tmp$SrsImgFolder <- exp_info$SrsImgFolder[i]
		tmp$col <- rainbow(20)[c(1,3,4,5,8,11,13,17,18)][(i-1)%%8+1]
		tmp$tx <- (1:dim(tmp)[1]) + curtx
		curtx <- max(tmp$tx)

		if(dim(result)[2]>0){
			nms1 <- names(tmp)[!(names(tmp)%in%names(result))]
			nms2 <- names(result)[!(names(result)%in%names(tmp))]
			if(length(nms1)>0){
				tdf = data.frame(matrix(NA,dim(result)[1],length(nms1)))
				names(tdf) = nms1
				result2 <- cbind(result,tdf)
			}else{
				result2 <- result
			}
			if(length(nms2)>0){
				tdf = data.frame(matrix(NA,dim(tmp)[1],length(nms2)))
				names(tdf) <- nms2
				tmp2 <- cbind(tmp,tdf)
			}else{
				tmp2 <- tmp
			}
			result <- rbind(result2,tmp2[,match(names(result2),names(tmp2))])
		}else{
			result <- tmp
		}
	}
	return(result)
}


read_vmDat <- function(filename,h=300){
	vmraw <- read.table(filename)
	vm <- data.frame(
		index=(1:length(vmraw[,1])),
		id=as.numeric(paste("0x",sub(",","",as.character(vmraw[,3])),sep="")),
		elapsed=vmraw[,5],
		timestamp=as.numeric(paste("0x",as.character(vmraw[,8]),sep=""))
		)

	len <- length(vm[,1])
	dif <- c(0,vm$id[2:len] - vm$id[1:(len-1)])
	jup <- vm[dif<(-40000),]
	vm$idcycle <- 0
	if(length(jup$index)>0){
		jup$clust <- 1
		if(length(jup$clust)>1){
			d = dist(jup$index)
		      clust = hclust(d,"single")
			jup$clust = cutree(clust,h=h)
		}
		for(cycle in 1:max(jup$clust)){
			jups <- jup[jup$clust==cycle,]
			if(length(jups[,1])==1){
				pos <- vm$index>=jups$index[1]
			}else{
				pos1 <- vm$index>=jups$index[1] & vm$index<jups$index[length(jups$index)] & vm$id<40000
				pos2 <- vm$index>=jups$index[length(jups$index)]
				pos <- pos1|pos2
			}
			vm$idcycle[pos] <- vm$idcycle[pos]+1
		}
	}
	vm$id2 <- vm$id + 2^16*vm$idcycle
	vm <- vm[order(vm$id2),]

	dif <- c(0,vm$timestamp[2:len] - vm$timestamp[1:(len-1)])
	jup <- vm[dif<(-2^28),]
	vm$timecycle <- 0
	if(length(jup$id2)>0){
		for(i in jup$id2){
			pos <- vm$id2>=i
			vm$timecycle[pos] <- vm$timecycle[pos]+1
		}
	}
	vm$timestamp2 <- vm$timestamp + 2^32*vm$timecycle - vm$timestamp[1]
	vm$time <- vm$timestamp2 / (50*10^6)
	vm$interval <- c(0,vm$time[2:len] - vm$time[1:(len-1)])*1000
	return(vm)
}

read_log <- function(filename){

# REF: ログの見方(単位はマイクロ秒)
#   id:          粒子 ID
#   abs_time:    速度情報を受信した時刻（基準）
#   1st_img:     最初の画像を受信した時刻 - 速度情報を受信した時刻
#   2nd_img:     ２番目の画像を受信した時刻 - 最初の画像を受信した時刻
#   3rd_img: ３番目の画像を受信した時刻 - ２番目の画像を受信した時刻
#   data_ready:  全画像データと速度情報が揃った時刻 - ３番目の画像を受信した時刻
#   proc_begin:  画像処理の開始時刻 - データが揃った時刻
#   proc_time:   画像処理時間
#   total:       画像処理の終了時刻 - abs_time
#   interval:    直前の画像処理の終了時刻との差

	inraw <- read.table(filename)
	colnames(inraw) <- c("log_id","log_st_time","log_img1","log_img2","log_img3","log_data_ready","log_proc_begin","log_proc_time","log_total_time","log_interval")
		# log_st_timeは起点となる時間（ミリ秒単位）
		# log_img1以降はlog_st_timeを起点とした差分時間（ミリ秒単位）
	return(inraw)
}

read_ilogDat <- function(filename){
	inraw <- read.table(filename)
	indat <- data.frame(
		id=inraw[,1],
		time=(inraw[,2]-inraw[1,2])/1000
		)
	indat <- indat[order(indat$time),]
	len <- length(indat[,1])
	indat$interval <- c(0,indat$time[2:len] - indat$time[1:(len-1)])
	indat$iddif <- c(1,indat$id[2:len] - indat$id[1:(len-1)])
	rollpoints <- (1:len)[indat$iddif<(-10000)]
	for(i in 1:length(rollpoints)){
		indat$iddif[rollpoints[i]] <- indat$id[rollpoints[i]]+(65535-indat$id[rollpoints[i]-1]+1)
	}
	indat$id2 <- cumsum(indat$iddif)
	indat$iddrop <- indat$iddif-1
	return(indat)
}

read_S5_data <- function(anal_srs_file){

	s_exists = file.exists(anal_srs_file)

	if(s_exists){
		tmp_s <- read_fujiDatSRS(anal_srs_file)
		tmp_s <- tmp_s[order(tmp_s$id),]
		names(tmp_s) = paste("s_",names(tmp_s),sep="")
		return(tmp_s)
	}
}

read_fujiDatSRS <- function(fujidat_file){
	result <- read.csv(fujidat_file,header=F,as.is=T)
	colnames(result) <- result[1,]
	result <- result[-1,]
	result <- result[,colnames(result)!="NA"]

	# indexの"experiment_finenum_imagenum"を分解
	a <- strsplit(as.character(result$index),"_")
	len <- length(a[[1]])
	result$filenum <- unlist(lapply(a,"[",len-1))
	result$imagenum <- unlist(lapply(a,"[",len))
	result <- data.frame(lapply(result[,-1],as.numeric))

	# Event Nuberプロット用のパラメータtx
	result$tx <- as.numeric(result$filenum)*max(as.numeric(result$imagenum))+as.numeric(result$imagenum)

	# ログ値の追加
	result$logint_CH0 <- log10(result$integral_CH0)
	result$logint_CH1 <- log10(result$integral_CH1)
	result$logint_CH2 <- log10(result$integral_CH2)
	result$logint_CH3 <- log10(result$integral_CH3)
	result$logave_CH0 <- log10(result$ave_CH0)
	result$logave_CH1 <- log10(result$ave_CH1)
	result$logave_CH2 <- log10(result$ave_CH2)
	result$logave_CH3 <- log10(result$ave_CH3)
	result$logmax_CH0 <- log10(result$max_CH0)
	result$logmax_CH1 <- log10(result$max_CH1)
	result$logmax_CH2 <- log10(result$max_CH2)
	result$logmax_CH3 <- log10(result$max_CH3)
	result$logint_CH0[is.na(result$logint_CH0)] <- 0.1
	result$logint_CH1[is.na(result$logint_CH1)] <- 0.1
	result$logint_CH2[is.na(result$logint_CH2)] <- 0.1
	result$logint_CH3[is.na(result$logint_CH3)] <- 0.1
	result$logave_CH0[is.na(result$logave_CH0)] <- 0.1
	result$logave_CH1[is.na(result$logave_CH1)] <- 0.1
	result$logave_CH2[is.na(result$logave_CH2)] <- 0.1
	result$logave_CH3[is.na(result$logave_CH3)] <- 0.1
	result$logmax_CH0[is.na(result$logmax_CH0)] <- 0.1
	result$logmax_CH1[is.na(result$logmax_CH1)] <- 0.1
	result$logmax_CH2[is.na(result$logmax_CH2)] <- 0.1
	result$logmax_CH3[is.na(result$logmax_CH3)] <- 0.1
	result$Sample <- ""
	result$col <- 1
	return(result)
}

