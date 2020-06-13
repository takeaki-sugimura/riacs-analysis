/**
 * @file srs_proc.cpp
 * @brief Offline sort test
 * @author Yasuhiro Fujiwaki, Takeaki Sugimura
 * @date 2019/3/1
 * Copyright (c) 2019 [ImPACT program] Planned serendipity. All rights reserved.
 */

#include <sys/stat.h>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "srs_file.h"

using namespace cv;
using namespace std;

extern "C" bool serendipiter5_image_sort_3t3l1(const cv::Mat&, const cv::Mat&, const cv::Mat&, const cv::Mat&, double (&cres)[16]);
extern "C" bool serendipiter5_image_sort_chlam(const cv::Mat&, const cv::Mat&, const cv::Mat&, const cv::Mat&, double (&cres)[16]);
extern "C" bool serendipiter5_image_sort_euglena(const cv::Mat&, const cv::Mat&, const cv::Mat&, const cv::Mat&, double (&cres)[16]);

int main(int argc, char **argv)
{

  if (argc != 3)
  {
    cout << "error!" << endl;
    cout << "Usage : " << argv[0] << " SRS-directory-path function-no(3t3l1:1, chlamydomonas:2, Euglena:3)" << endl;
  }
  int fnum = atoi(argv[2]);

  const int kImgMatCol = 40;
  const int kImgMatRow = 25;

  // input paths
  string SRS_dir_path(argv[1]);
  int dir_name_head_location = SRS_dir_path.find_last_of("/") + 1;
  string SRS_file_name = SRS_dir_path.substr(dir_name_head_location, SRS_dir_path.length() - dir_name_head_location) + ".srs";
  string file_path = SRS_dir_path + "/" + SRS_file_name;

  // output paths & dir.
  string img_dir_path = SRS_dir_path + "/srsimg";
  struct stat buf;
  if (stat(img_dir_path.c_str(), &buf) == -1)
  {
    mkdir(img_dir_path.c_str(), 0777);
  }
  string result_path = SRS_dir_path + "/result_s.csv";

  // open SRS file
  SRSFile SRS_file(file_path);

  // prepare image matrix
  vector<Mat> src_mat_imgs;
  int resized_image_size_x = SRS_file.image_size_x();
  int resized_image_size_y = round(SRS_file.channel_number() * 3.0);
  Size mat_imgs_size(resized_image_size_x * kImgMatCol,
                     resized_image_size_y * kImgMatRow);
  for (int i = 0; i < SRS_file.color_number(); i++) {
    src_mat_imgs.push_back(Mat(mat_imgs_size, CV_8UC1, Scalar(0)));
  }

  // open result file
  ofstream result_file(result_path, ofstream::trunc);
  if (result_file.fail()) {
    std::cout << "error! Failed to open \"" << result_path << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }

  int proc_check = 0;
  cout << "processing " << SRS_file.file_path() << " ..." << flush;
  if (fnum == 1) {
	result_file << "index,area,ave0,ave1,ave2,int0,int1,int2,std0,std1,std2,sort"<< endl;	  
  }
  else if (fnum == 2) {
	result_file << "index,area,ave0,ave1,ave2,int0,int1,int2,na,na,na,sort"<< endl;	  	  
  }
  else {
	result_file << "index,area,ave0,ave1,ave2,int0,int1,int2,std0,std1,std2,sort"<< endl;	  
  }

  for (int i = 0; i < SRS_file.image_number(); i++) {
    int image_mat_number = i / (kImgMatCol * kImgMatRow);
    int image_mat_location = i % (kImgMatCol * kImgMatRow);
    vector<Mat> src_imgs;
    SRS_file.ReadImageAllChannel(src_imgs, i);
	
    // Sort function
    cv::Mat srs_img[4];
    double result[16] = {};
    bool sort = false;
    for (int j=0; j<4; j++) {
      src_imgs.at(j).copyTo(srs_img[j]);
    }
	if (fnum == 1) {
		sort = serendipiter5_image_sort_3t3l1(srs_img[0], srs_img[1], srs_img[2], srs_img[3], result); 		
	}
	else if (fnum == 2) {
		sort = serendipiter5_image_sort_chlam(srs_img[0], srs_img[1], srs_img[2], srs_img[3], result); 		
	}
	else {
		sort = serendipiter5_image_sort_euglena(srs_img[0], srs_img[1], srs_img[2], srs_img[3], result); 				
	}

    // paste images to image matrix
    Rect roi = Rect((image_mat_location % kImgMatCol) * resized_image_size_x,
                    (image_mat_location / kImgMatCol) * resized_image_size_y,
                    resized_image_size_x,
                    resized_image_size_y);
    for (int j = 0; j < SRS_file.color_number(); j++)
    {
      cv::Mat resized_img;
      cv::resize(srs_img[j], resized_img, resized_img.size(), 1, 3.0, cv::INTER_LINEAR);
      resized_img.convertTo(src_mat_imgs.at(j)(roi), CV_8UC1);
    }
    result_file << image_mat_number << "_" << image_mat_location << ","
                << result[0] << "," << result[1] << "," << result[2] << "," << result[3] << "," << result[4] << ","
				<< result[5] << "," << result[6] << "," << result[7] << "," << result[8] << "," << result[9] << "," << sort << endl;

    // export images
    if (image_mat_location == kImgMatCol * kImgMatRow - 1
        || i == SRS_file.image_number() - 1)
    {
      for (int j = 0; j < SRS_file.color_number(); j++)
      {
        ostringstream mat_img_path;
        mat_img_path << img_dir_path << "/"
                     << image_mat_number << "_CH" << j << ".png";
        imwrite(mat_img_path.str(), src_mat_imgs.at(j));
        src_mat_imgs.at(j) = Mat(mat_imgs_size, CV_8UC1, Scalar(0));
      }
    }

    // display progress
    if ((i * 10) / SRS_file.image_number() > proc_check)
    {
      cout << (i * 10) / SRS_file.image_number() << "0%..." << flush;
      proc_check = (i * 10) / SRS_file.image_number();
    }
  }
  cout << "done" << endl;

  return 0;
}
