#ifndef SRS_FILE_H_
#define SRS_FILE_H_

#include <fstream>
#include <opencv2/core.hpp>

class SRSFile
{
public:
  SRSFile(std::string);
  SRSFile();
  ~SRSFile();

  std::string file_path() { return file_path_; }
  int color_number() { return color_number_; }
  int image_number() { return image_number_; }
  int image_size_x() { return image_size_x_; }
  int channel_number() { return channel_number_; }
  int header_number() { return kHeaderNumber_; }

  void open(std::string);
  void close();
  cv::Mat ReadImage(int, int);
  void ReadImageAllChannel(std::vector<cv::Mat> &, int);
  std::string InfoHeader();
  std::string InfoString();

private:
  std::string file_path_;
  std::ifstream SRS_file_;
  int color_number_;
  int image_number_;
  int image_size_x_;
  int channel_number_;
  static const int kHeaderNumber_ = 4;
};

#endif /* end of include guard: SRS_FILE_H_ */
