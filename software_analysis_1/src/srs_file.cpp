#include "srs_file.h"
#include <iostream>
#include <opencv2/imgproc.hpp>

#include <bitset>

namespace
{
enum SRSHeaderKind
{
  SRS_COLOR_NUMBER,
  SRS_IMAGE_NUMBER,
  SRS_CHANNEL_NUMBER,
  SRS_IMAGE_SIZE_X
};

int read_SRS_header(std::ifstream &SRS_file, SRSHeaderKind header_kind)
{
  char buf[4];
  int data;
  SRS_file.seekg(header_kind * 4);
  SRS_file.read(buf, 4);

  data = static_cast<uchar>(buf[0]) << 24 | static_cast<uchar>(buf[1]) << 16
         | static_cast<uchar>(buf[2]) << 8 | static_cast<uchar>(buf[3]);

  return data;
}
} // namespace

SRSFile::SRSFile(std::string file_path)
{
  open(file_path);
}

SRSFile::SRSFile()
    : file_path_(std::string()),
      SRS_file_(std::ifstream()),
      color_number_(-1),
      image_number_(-1),
      image_size_x_(-1),
      channel_number_(-1)
{
}

SRSFile::~SRSFile()
{
  SRS_file_.close();
}

void SRSFile::open(std::string file_path)
{
  if (SRS_file_.is_open())
    close();

  file_path_ = file_path;

  SRS_file_.open(file_path, std::ifstream::binary);
  if (SRS_file_.fail())
  {
    std::cout << "Failed to open \"" << file_path_ << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }
  color_number_ = read_SRS_header(SRS_file_, SRS_COLOR_NUMBER);
  image_number_ = read_SRS_header(SRS_file_, SRS_IMAGE_NUMBER);
  channel_number_ = read_SRS_header(SRS_file_, SRS_CHANNEL_NUMBER);
  image_size_x_ = read_SRS_header(SRS_file_, SRS_IMAGE_SIZE_X);
}

void SRSFile::close()
{
  file_path_.clear();
  SRS_file_.close();
  color_number_ = -1;
  image_number_ = -1;
  image_size_x_ = -1;
  channel_number_ = -1;
}

cv::Mat SRSFile::ReadImage(int target_image_number, int target_color_number)
{
  assert(target_image_number >= 0 && target_image_number < image_number_);
  assert(target_color_number >= 0 && target_color_number < color_number_);

  assert(SRS_file_.is_open());
  cv::Mat raw_img(cv::Size(image_size_x_, channel_number_),
                  CV_16SC1, cv::Scalar(0));
  unsigned long long image_position = (target_image_number + target_color_number
                                       * static_cast<unsigned long long>(image_number_))
                                       * image_size_x_ * channel_number_ * 2
                                       + kHeaderNumber_ * 4;
  SRS_file_.seekg(image_position);
  assert(SRS_file_);
  for (int y = 0; y < channel_number_; y++)
  {
    for (int x = 0; x < image_size_x_; x++)
    {
      char buf[2];
      SRS_file_.read(buf, 2);
      raw_img.at<short>(y, x) = static_cast<uchar>(buf[0]) << 8
                                | static_cast<uchar>(buf[1]);
    }
  }
  raw_img.convertTo(raw_img, CV_8UC1);
  return raw_img.clone();
}

void SRSFile::ReadImageAllChannel(std::vector<cv::Mat> &dst,
                                  int target_image_number)
{
  dst.clear();
  for (int i = 0; i < color_number_; i++)
  {
    cv::Mat buf = ReadImage(target_image_number, i);
    dst.push_back(buf.clone());
  }
}

std::string SRSFile::InfoHeader()
{
  return "color_number,image_number,image_size_x,channel_number";
}

std::string SRSFile::InfoString()
{
  std::ostringstream info_string;
  info_string << color_number_ << ","
              << image_number_ << ","
              << image_size_x_ << ","
              << channel_number_;
  return info_string.str();
}
