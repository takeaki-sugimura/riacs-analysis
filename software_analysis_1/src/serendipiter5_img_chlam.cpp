/** 
 * @brief Sorting program for Chlamydomonas 
 * @author Yasuhiro Fujiwaki, Takeaki Sugimura
 * @date 2019/3/1
 * Copyright (c) 2019 [ImPACT program] Planned serendipity. All rights reserved.
 */

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

extern "C" {
bool serendipiter5_image_sort_chlam(const cv::Mat& srs_ch0, const cv::Mat& srs_ch1,
									const cv::Mat& srs_ch2, const cv::Mat& srs_ch3,
									double (&cres)[16]) 
{
  bool result = false;

  if(srs_ch1.empty()){
    return result;
  }
  double resize_ratio = 1.0/0.8;  // 1.0 x 0.8um / pixel
  Mat norm_srs_ch0, norm_srs_ch1, norm_srs_ch2, norm_srs_ch3;
  resize(srs_ch0, norm_srs_ch0, norm_srs_ch0.size(), 1, resize_ratio, cv::INTER_LINEAR); 
  resize(srs_ch1, norm_srs_ch1, norm_srs_ch1.size(), 1, resize_ratio, cv::INTER_LINEAR); 
  resize(srs_ch2, norm_srs_ch2, norm_srs_ch2.size(), 1, resize_ratio, cv::INTER_LINEAR); 
  resize(srs_ch3, norm_srs_ch3, norm_srs_ch3.size(), 1, resize_ratio, cv::INTER_LINEAR);
  Mat unorm_srs_ch0( Size(norm_srs_ch0.cols, norm_srs_ch0.rows), CV_16UC1);
  Mat unorm_srs_ch1( Size(norm_srs_ch1.cols, norm_srs_ch1.rows), CV_16UC1);
  Mat unorm_srs_ch2( Size(norm_srs_ch2.cols, norm_srs_ch2.rows), CV_16UC1);
  Mat unorm_srs_ch3( Size(norm_srs_ch3.cols, norm_srs_ch3.rows), CV_16UC1);
  norm_srs_ch0.convertTo(unorm_srs_ch0, CV_16UC1);
  norm_srs_ch1.convertTo(unorm_srs_ch1, CV_16UC1);
  norm_srs_ch2.convertTo(unorm_srs_ch2, CV_16UC1);
  norm_srs_ch3.convertTo(unorm_srs_ch3, CV_16UC1);
  // Making mask
  Mat src_img( Size(unorm_srs_ch0.cols, unorm_srs_ch0.rows), CV_16UC1);
  src_img = (unorm_srs_ch0 + unorm_srs_ch1 + unorm_srs_ch2 + unorm_srs_ch3) / 4;   // (CH0+CH1+CH2+CH3)/4
  cv::Rect noise_cut_roi(0, 0, src_img.cols, src_img.rows);  
  // Parameters
  int min_normalize_scale = 0;
  int morphology_kernel_size = 2;
  int morphology1st_close_iteration = 3;
  int morphology1st_open_iteration = 3;
  int morphology2nd_close_iteration = 5;
  int morphology2nd_open_iteration = 5;
  // unsharping filter
  cv::Mat filtered_img1;
  float unsharp_kernel_data[] = {
      -1 / 9.0f,
      -1 / 9.0f,
      -1 / 9.0f,
      -1 / 9.0f,
      17 / 9.0f,
      -1 / 9.0f,
      -1 / 9.0f,
      -1 / 9.0f,
      -1 / 9.0f};
  cv::Mat unsharp_kernel(3, 3, CV_32F, unsharp_kernel_data);
  filter2D(src_img, filtered_img1, -1, unsharp_kernel);
  // remove background
  cv::Mat filtered_img2 = filtered_img1.clone();
  cv::Mat avg_row;
  reduce(filtered_img2, avg_row, 1, cv::REDUCE_AVG, CV_32F);
  for (int i = 0; i < filtered_img2.rows; i++)
  {
    int ave_recalc = 0;
    int k = 0;
    ushort thresh_recalc = static_cast<ushort>(avg_row.at<float>(i, 0) * morphology2nd_open_iteration / 100);
    for (int j = 0; j < filtered_img2.cols; j++)
    {
      if (filtered_img2.at<ushort>(i, j) < thresh_recalc)
      {
        ave_recalc += filtered_img2.at<ushort>(i, j);
        k++;
      }
    }
    if (k != 0)
    {
      ave_recalc /= k;
    }
    cv::Rect row_roi(0, i, filtered_img2.cols, 1);
    if (k < 0)
    {
      filtered_img2(row_roi) -= cv::min(ave_recalc, filtered_img2(row_roi));
    }
    else
    {
      filtered_img2(row_roi) -= cv::min(avg_row.at<float>(i, 0), filtered_img2(row_roi));
    }
  }
  // calculate normalize scale
  double min_val, max_val;
  minMaxLoc(filtered_img2, &min_val, &max_val);
  int channels[] = { 0 };
  cv::Mat img_hist;
  int bin_nums[] = { 65536 };
  float range[] = { 0, 65535 };
  const float * ranges[] = { range };
  calcHist(&filtered_img2, 1, channels, cv::Mat(), img_hist, 1, bin_nums, ranges);
  int peak_val;
  minMaxIdx(img_hist, NULL, NULL, NULL, &peak_val);
  int normalize_scale = ((max_val - peak_val) > (peak_val - min_val)) ? max_val - peak_val : peak_val - min_val;
  if (normalize_scale < min_normalize_scale) {
    normalize_scale = min_normalize_scale;
  }
  cv::Mat normalized_img = 65535 * abs(filtered_img2 - peak_val) / normalize_scale;
  // threshold
  cv::Mat zero_thresh_img;
  cv::Mat thresh_img;
  normalized_img.convertTo(zero_thresh_img, CV_8UC1, (double)255/65535);
  threshold(zero_thresh_img, thresh_img, 0, 255, cv::THRESH_OTSU);
  // Morphology
  int extend_edge_size = 40;
  cv::Mat morphology_img1_extended(cv::Size((thresh_img.cols + extend_edge_size*2), (thresh_img.rows + extend_edge_size*2)),
                                   CV_8UC1, cv::Scalar(0));
  cv::Mat morphology_img1(morphology_img1_extended, cv::Rect(extend_edge_size, extend_edge_size, thresh_img.cols, thresh_img.rows));
  thresh_img.copyTo(morphology_img1);
  cv::Mat kernel = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(morphology_kernel_size, morphology_kernel_size));
  morphologyEx(morphology_img1_extended, morphology_img1_extended, cv::MORPH_CLOSE, kernel,
               cv::Point(-1, -1), morphology1st_close_iteration);
  morphologyEx(morphology_img1_extended, morphology_img1_extended, cv::MORPH_OPEN, kernel,
               cv::Point(-1, -1), morphology1st_open_iteration);
  // fill
  std::vector<std::vector<cv::Point> > contours;
  findContours(morphology_img1, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
  cv::Mat filled_img(morphology_img1.size(), CV_8UC1, cv::Scalar(0));
  drawContours(filled_img, contours, -1, cv::Scalar(255), cv::FILLED);
  // morphology
  cv::Mat morphology_img2_extended(cv::Size((filled_img.cols + extend_edge_size*2),
                                   (filled_img.rows + extend_edge_size*2)),
                                   CV_8UC1, cv::Scalar(0));
  cv::Mat morphology_img2(morphology_img2_extended,
                          cv::Rect(extend_edge_size, extend_edge_size,
                                   filled_img.cols, filled_img.rows));
  filled_img.copyTo(morphology_img2);
  morphologyEx(morphology_img2_extended, morphology_img2_extended, cv::MORPH_OPEN, kernel,
               cv::Point(-1, -1), morphology2nd_open_iteration);
  morphologyEx(morphology_img2_extended, morphology_img2_extended, cv::MORPH_CLOSE, kernel,
               cv::Point(-1, -1), morphology2nd_close_iteration);

  // find max mask
  findContours(filled_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
  double max_mask_area = 0;
  int max_mask_index = -1;
  for(uint i = 0; i < contours.size(); i++){
    double area = contourArea(contours.at(i));
    if(max_mask_area <= area){
      max_mask_area = area;
      max_mask_index = i;
    }
  }
  cv::Mat max_picked_img(filled_img.size(), CV_8UC1, cv::Scalar(0));
  if (max_mask_index >= 0) {
    drawContours(max_picked_img, contours, max_mask_index, cv::Scalar(255), cv::FILLED);
  }
  // change mask size to match src_img
  cv::Mat mask_img(src_img.size(), CV_8UC1, cv::Scalar(0));
  cv::Mat mask_img_roi(mask_img, noise_cut_roi);
  max_picked_img.copyTo(mask_img_roi);
  double base[3][4] = {{0.20238095238095238, 0.27380952380952384, 0.2619047619047619, 0.2619047619047619,},
                       {0.22988505747126436, 0.25287356321839083, 0.26436781609195403, 0.25287356321839083},
                       {0.2077922077922078, 0.24675324675324675, 0.3116883116883117, 0.23376623376623376}}; 
  cv::Mat baseMat = cv::Mat(cv::Size(4,3), CV_64FC1, base);
  int hxw = src_img.cols * src_img.rows;
  cv::Mat im1d = cv::Mat(hxw, 4, CV_64FC1); 
  double max_Est, min_Est;
  // Remove background
  cv::Scalar bglevel;
  cv::Mat img_nobg[4];
  cv::Mat imgroi_D = cv::Mat(hxw, 4, CV_64FC1);
  cv::Mat imgroi_EstCom;
  bglevel = cv::mean(unorm_srs_ch0);
  img_nobg[0] = unorm_srs_ch0 - (int)bglevel[0];
  bglevel = cv::mean(unorm_srs_ch1);
  img_nobg[1] = unorm_srs_ch1 - (int)bglevel[0];
  bglevel = cv::mean(unorm_srs_ch2);
  img_nobg[2] = unorm_srs_ch2 - (int)bglevel[0];
  bglevel = cv::mean(unorm_srs_ch3);
  img_nobg[3] = unorm_srs_ch3 - (int)bglevel[0];
  for (int i=0; i<4; i++) {
    im1d = img_nobg[i].reshape(1, hxw);
    im1d.copyTo(imgroi_D(Rect(i,0,1,hxw)));
  }
  imgroi_EstCom = imgroi_D * baseMat.inv(cv::DECOMP_SVD);
  minMaxLoc(imgroi_EstCom, &min_Est, &max_Est);
  cv::Mat tmpEst[4];
  cv::Mat imgroi_Est[3];
  for (int j=0; j<imgroi_EstCom.cols; j++) {
    tmpEst[0] = Mat(imgroi_EstCom, Rect(j,0,1,hxw));
    tmpEst[0].copyTo(tmpEst[1]);
    tmpEst[2] = tmpEst[1].reshape(1, src_img.rows);
    tmpEst[3] = tmpEst[2] / max_Est;  
    tmpEst[3].copyTo(imgroi_Est[j]); 
  }
  int tran_area = 0;
  double ave_est0 = 0;
  double ave_est1 = 0;
  double ave_est2 = 0;
  double integral_est0 = 0;
  double integral_est1 = 0;
  double integral_est2 = 0;
  cv::Mat masked_est0(cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  cv::Mat masked_est1(cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  cv::Mat masked_est2(cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  for (int i=0; i < src_img.cols; i++) {
    for (int j=0; j < src_img.rows; j++) {
      masked_est0.at<double>(j,i) = (double) imgroi_Est[0].at<double>(j,i) * (double) mask_img.at<uchar>(j,i) / 255;
      masked_est1.at<double>(j,i) = (double) imgroi_Est[1].at<double>(j,i) * (double) mask_img.at<uchar>(j,i) / 255;
      masked_est2.at<double>(j,i) = (double) imgroi_Est[2].at<double>(j,i) * (double) mask_img.at<uchar>(j,i) / 255;
      tran_area += mask_img.at<uchar>(j,i);
      integral_est0 += masked_est0.at<double>(j,i);
      integral_est1 += masked_est1.at<double>(j,i);
	  integral_est2 += masked_est2.at<double>(j,i);
    }
  }
  if (tran_area > 1) {
    tran_area = cvRound( tran_area / 255 );
    ave_est0 = integral_est0 / tran_area;
    ave_est1 = integral_est1 / tran_area;
	ave_est2 = integral_est2 / tran_area;
  }
  cres[0] = tran_area;
  cres[1] = ave_est0;
  cres[2] = ave_est1;
  cres[3] = ave_est2;
  cres[4] = integral_est0;
  cres[5] = integral_est1;
  cres[6] = integral_est2;
  
  if ( (tran_area > 20) & (ave_est0 > 0.05) ) {
    result = true;
  }
  return result;
}
  
};

