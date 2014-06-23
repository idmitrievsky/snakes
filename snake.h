//
//  snake.h
//  snakes
//
//  Created by Ivan Dmitrievsky on 11/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#ifndef __snakes__snake__
#define __snakes__snake__

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <string>

typedef cv::Mat Image;

class Snake {
  double tension;
  double stiffness;

  double line_weight;
  double edge_weight;
  double term_weight;

  bool fixed;
  bool closed;

  int implicit;

  double atom;
  double tick;

  double threshold;
  
  Image img;
  std::vector<uchar> raw_img;
  std::size_t raw_img_size;
  std::pair<Image, Image> grad;
  Image hess;
  cv::Mat pentamat;

  std::vector<double> xs;
  std::vector<double> ys;

  cv::VideoCapture vid;
  
  void update_raw_img();
public:
  Snake(std::string json_file_path);

  void set_xs(std::vector<double> _xs);
  void set_ys(std::vector<double> _ys);
  void set_pentamat();
  bool next_frame();
  
  std::vector<double> get_xs() const;
  std::vector<double> get_ys() const;
  void *get_raw_img();
  std::size_t get_raw_img_size();

  bool is_closed();
  int get_implicit();

  bool update();
  void print_and_save(std::string const &output_file_path);
};

#endif /* defined(__snakes__snake__) */
