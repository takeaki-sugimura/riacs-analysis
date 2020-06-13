/**
 * @brief Sorting program for Serendipiter 5.0 
  *@author Takahumi Kobayashi, Takeaki Sugimura
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
bool serendipiter5_image_sort_euglena(const cv::Mat& srs_ch0, const cv::Mat& srs_ch1,
										const cv::Mat& srs_ch2, const cv::Mat& srs_ch3,
										double (&cres)[16]) 
{
  bool result = false;

  if(srs_ch1.empty()){
    return result;
  }
  double resize_ratio = 1.0;
  Mat unorm_srs_ch0( Size(srs_ch0.cols, srs_ch0.rows), CV_16UC1);
  Mat unorm_srs_ch1( Size(srs_ch1.cols, srs_ch1.rows), CV_16UC1);
  Mat unorm_srs_ch2( Size(srs_ch2.cols, srs_ch2.rows), CV_16UC1);
  Mat unorm_srs_ch3( Size(srs_ch3.cols, srs_ch3.rows), CV_16UC1);
  Mat dbl_srs_ch0( Size(srs_ch0.cols, srs_ch0.rows), CV_64FC1);
  Mat dbl_srs_ch1( Size(srs_ch1.cols, srs_ch1.rows), CV_64FC1);
  Mat dbl_srs_ch2( Size(srs_ch2.cols, srs_ch2.rows), CV_64FC1);
  Mat dbl_srs_ch3( Size(srs_ch3.cols, srs_ch3.rows), CV_64FC1);
  srs_ch0.convertTo(dbl_srs_ch0, CV_64FC1);
  srs_ch1.convertTo(dbl_srs_ch1, CV_64FC1);
  srs_ch2.convertTo(dbl_srs_ch2, CV_64FC1);
  srs_ch3.convertTo(dbl_srs_ch3, CV_64FC1);
  srs_ch0.convertTo(unorm_srs_ch0, CV_16UC1);
  srs_ch1.convertTo(unorm_srs_ch1, CV_16UC1);
  srs_ch2.convertTo(unorm_srs_ch2, CV_16UC1);
  srs_ch3.convertTo(unorm_srs_ch3, CV_16UC1);
  // Making mask
  cv::Mat src_img( cv::Size(unorm_srs_ch0.cols, unorm_srs_ch0.rows), CV_16UC1);
  src_img = unorm_srs_ch3;
  cv::Rect noise_cut_roi(0, 0, src_img.cols, src_img.rows); 
  // Remove background
  cv::Mat dbl_srs( cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  src_img.convertTo(dbl_srs, CV_64FC1);
  double mask_bglevel[24] = {21.6,23.1,24.6,22.7,24.8,25.9,24.8,23.3,24.2,23.7,22.4,22.7,22,20.7,19.3,17.6,15.7,16.6,15.6,14.4,12.8,11.6,10.3,8.5};
  cv::Mat mask_img_nobg( cv::Size(src_img.cols, src_img.rows), CV_64FC1);
   for (int i=0; i < src_img.cols; i++) {
    for (int j=0; j < src_img.rows; j++) {
      mask_img_nobg.at<double>(j,i) = dbl_srs.at<double>(j,i) - mask_bglevel[j];
    }
  }
  // Resize
  cv::Mat img_nobg_rs;
  double mresize_ratio = 3.0;
  resize(mask_img_nobg, img_nobg_rs, img_nobg_rs.size(), 1, mresize_ratio, cv::INTER_LINEAR); 
  // Bottom hat filter (Closing - Original)
  cv::Mat img_bthat;
  cv::Mat bthat_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10,10), cv::Point(5,5));
  cv::morphologyEx(img_nobg_rs, img_bthat, cv::MORPH_BLACKHAT, bthat_element);
  // Gausian filter (13x13)
  cv::Mat img_gfil;
  double sigma = 1.4142;
  cv::GaussianBlur(img_bthat, img_gfil, cv::Size(13,13), sigma, sigma);
  double min_val_tmp, max_val_tmp;
  minMaxLoc(img_gfil, &min_val_tmp, &max_val_tmp);
  cv::Mat normalized_img_tmp = 255 * ((img_gfil - min_val_tmp) / (max_val_tmp - min_val_tmp));
  cv::Mat norm8b_img_tmp;
  normalized_img_tmp.convertTo(norm8b_img_tmp, CV_8UC1);
  // Canny edge detection
  double min_val = 0;
  double max_val = 30.0;
  double canny_threshold1 = 60;
  double canny_threshold2 = 180;
  minMaxLoc(img_gfil, &min_val, &max_val);
  if ((max_val - min_val) < 15) {
    max_val = min_val + 15;
  }
  cv::Mat normalized_img = (img_gfil - min_val) / (max_val - min_val);
  cv::Mat norm8b_img;
  normalized_img.convertTo(norm8b_img, CV_8UC1, (double)255);
  cv::Mat canny_img;
  canny_threshold2 = 0.7 * 255;
  canny_threshold1 = canny_threshold2 * 0.6;
  cv::Canny(norm8b_img, canny_img, canny_threshold1, canny_threshold2);
  // Dilation
  cv::Mat dil_img;
  cv::Mat dil_element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5), cv::Point(3,3));
  cv::dilate(canny_img, dil_img, dil_element);
  // Fill
  cv::Mat flood_img = dil_img.clone();
  cv::floodFill(flood_img, cv::Point(0,0), cv::Scalar(255));
  cv::Mat flood_inv;
  cv::bitwise_not(flood_img, flood_inv);
  cv::Mat fil_img = (dil_img | flood_inv);
  // Find contours
  std::vector<std::vector<cv::Point> > contours;
  findContours(fil_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
  // Filter by area
  int j=0;
  for (std::vector<std::vector<cv::Point>>::iterator it = contours.begin(); it!=contours.end(); ) {
    if (it->size() < 30) {
      it=contours.erase(it);
    }
    else {
      ++it;
      ++j;
    }
  }
  // Convexhull
  cv::Mat filled_img(fil_img.size(), CV_8UC1, cv::Scalar(0));
  if (contours.size() > 1) {
    for (int i=1; i<contours.size(); i++) {
      contours[0].insert(contours[0].end(), contours[i].begin(), contours[i].end());
    }
  }
  if (j > 0) {
    std::vector<std::vector<cv::Point> >hull(1);
    cv::convexHull( cv::Mat(contours[0]), hull[0], false);     
    cv::drawContours(filled_img, hull, -1, cv::Scalar(255), cv::FILLED);
  }
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
  cv::Mat cmask_img(src_img.size(), CV_8UC1, cv::Scalar(0));
  double cresize_ratio = 1/3.0;
  resize(max_picked_img, cmask_img, cmask_img.size(), 1, cresize_ratio, cv::INTER_LINEAR); 
  // For 12C/13C Euglena （C1:12C_paramylon,C2:Chlorophyl,C3:13C_paramylon）
  double base[3][4] = {{0.2025, 0.2658, 0.3038, 0.2278},
                       {0.2272, 0.2613, 0.2614, 0.2500},
                       {0.2025, 0.3038, 0.2658, 0.2278}}; 
  cv::Mat baseMat = cv::Mat(cv::Size(4,3), CV_64FC1, base);
  int hxw = src_img.cols * src_img.rows;
  cv::Mat im1d = cv::Mat(hxw, 4, CV_64FC1); // Define a matrix(3840 x 4), double.
  double max_Est, min_Est;
  // Remove background
  cv::Mat img_nobg[4];
  cv::Mat imgroi_D = cv::Mat(hxw, 4, CV_64FC1);
  cv::Mat imgroi_EstCom;
  img_nobg[0] = dbl_srs_ch0;
  img_nobg[1] = dbl_srs_ch1;
  img_nobg[2] = dbl_srs_ch2;
  img_nobg[3] = dbl_srs_ch3;
  double ch0_bglevel[24] = {18.3,20.1,20.3,19.9,20.8,21.6,21.1,19.1,21.1,19.6,19.9,19,19,17.8,16.8,15.8,14.5,14.7,13.1,13.4,12.1,10.5,10.4,9.1};
  double ch1_bglevel[24] = {18.5,20.6,20.9,20.4,21.1,22.5,21.8,20,20.9,20.8,20.4,19.8,19.7,19.5,17.4,17.4,15.6,15.9,15.1,14.2,12.8,12.4,10.4,10.4};
  double ch2_bglevel[24] = {19.2,21.4,22.9,21,20.8,22.9,21.9,20.1,21.3,21,20.9,20.2,21.1,19.9,18.9,17.3,16.8,16.4,16,14.5,13.5,12.3,11.4,9.2};
  double ch3_bglevel[24] = {21.6,23.1,24.6,22.7,24.8,25.9,24.8,23.3,24.2,23.7,22.4,22.7,22,20.7,19.3,17.6,15.7,16.6,15.6,14.4,12.8,11.6,10.3,8.5};
  for (int i=0; i < src_img.cols; i++) {
    for (int j=0; j < src_img.rows; j++) {
      img_nobg[0].at<double>(j,i) = (double)unorm_srs_ch0.at<ushort>(j,i) - ch0_bglevel[j];
      img_nobg[1].at<double>(j,i) = (double)unorm_srs_ch1.at<ushort>(j,i) - ch1_bglevel[j];
      img_nobg[2].at<double>(j,i) = (double)unorm_srs_ch2.at<ushort>(j,i) - ch2_bglevel[j];
      img_nobg[3].at<double>(j,i) = (double)unorm_srs_ch3.at<ushort>(j,i) - ch3_bglevel[j];
    }
  }
  for (int i=0; i<4; i++) {
    im1d = img_nobg[i].reshape(1, hxw);
    im1d.copyTo(imgroi_D(Rect(i,0,1,hxw)));
  }
  imgroi_EstCom = imgroi_D * baseMat.inv(cv::DECOMP_SVD); // EstCom = D * pinv(base).
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
  double std_est0 = 0;
  double std_est1 = 0;
  double std_est2 = 0;
  cv::Mat masked_est0(cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  cv::Mat masked_est1(cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  cv::Mat masked_est2(cv::Size(src_img.cols, src_img.rows), CV_64FC1);
  for (int i=0; i < src_img.cols; i++) {
    for (int j=0; j < src_img.rows; j++) {
      masked_est0.at<double>(j,i) = (double) imgroi_Est[0].at<double>(j,i) * (double) cmask_img.at<uchar>(j,i) / 255;
      masked_est1.at<double>(j,i) = (double) imgroi_Est[1].at<double>(j,i) * (double) cmask_img.at<uchar>(j,i) / 255;
      masked_est2.at<double>(j,i) = (double) imgroi_Est[2].at<double>(j,i) * (double) cmask_img.at<uchar>(j,i) / 255;
      tran_area += cmask_img.at<uchar>(j,i);
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
    cv::Mat ave_buf, std_buf;
    cv::meanStdDev(masked_est0, ave_buf, std_buf, cmask_img);
    std_est0 = std_buf.at<double>(0,0);
    cv::meanStdDev(masked_est1, ave_buf, std_buf, cmask_img);
    std_est1 = std_buf.at<double>(0,0);
	cv::meanStdDev(masked_est2, ave_buf, std_buf, cmask_img);
    std_est2 = std_buf.at<double>(0,0);
  }
  cres[0] = tran_area;
  cres[1] = ave_est0;
  cres[2] = ave_est1;
  cres[3] = ave_est2;
  cres[4] = integral_est0;
  cres[5] = integral_est1;
  cres[6] = integral_est2;
  cres[7] = std_est0;
  cres[8] = std_est1;
  cres[9] = std_est2;
  if ( (tran_area > 500) & (tran_area < 2000) & (ave_est2 > 0.02 + 1.0 * ave_est0) ) {
    result = true;
  }
  return result;
}
  
};

