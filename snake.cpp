//
//  snake.cpp
//  snakes
//
//  Created by Ivan Dmitrievsky on 11/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#include <utility>
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include "json11.hpp"
#include "snake.h"

static Image invert_image(Image const &img) {
  Image inverted_img;
  cv::subtract(cv::Scalar::all(255), img, inverted_img);
  return inverted_img;
}

static void save_double_heat_map(Image const &img, std::string const &path) {
  double min, max;
  cv::minMaxLoc(img, &min, &max);

  if (std::abs(min) > std::abs(max)) {
    max = min;
  }
  max = std::abs(max);

  Image scaled_img = (img / max) * 255;

  int nrows = scaled_img.rows, ncols = scaled_img.cols;

  Image heat_map(nrows, ncols, CV_8UC3);

  for (int i = 0; i < nrows; ++i) {
    for (int j = 0; j < ncols; ++j) {
      short val = (short)scaled_img.at<double>(i, j);
      if (val > 0) {
        heat_map.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, val);
      } else if (scaled_img.at<double>(i, j) < 0) {
        heat_map.at<cv::Vec3b>(i, j) = cv::Vec3b(-val, 0, 0);
      } else {
        heat_map.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0);
      }
    }
  }

  cv::imwrite(path, heat_map);
}

static void abs_threshold(Image img, double threshold) {
  double min, max;
  cv::minMaxLoc(img, &min, &max);

  if (std::abs(min) > std::abs(max)) {
    max = min;
  }
  max = std::abs(max);
  for (int i = 0; i < img.rows; ++i) {
    for (int j = 0; j < img.cols; ++j) {
      if (std::abs(img.at<double>(i, j)) < threshold * max / 255) {
        img.at<double>(i, j) = 0;
      }
    }
  }
}

static void line(Image &img, int start_x, int start_y, int end_x, int end_y) {
  int thickness = 1, line_type = 8;
  cv::Point start = cv::Point(start_x, start_y), end = cv::Point(end_x, end_y);
  cv::Scalar red(0, 0, 255);

  cv::line(img, start, end, red, thickness, line_type);
}

static void filled_circle(Image &img, int center_x, int center_y, int rad) {
  int thickness = 1, line_type = 8;
  cv::Point center = cv::Point(center_x, center_y);
  cv::Scalar red(0, 0, 255);

  cv::circle(img, center, rad, red, thickness, line_type);
}

/**
 *  Calculates img gradient and hessian
 *
 *  @param img             source image
 *  @param gauss           times gradient gets blurred
 *  @param hess            calculate hessian or not
 *
 *  @return pair of images, each representing one of gradient components
 */
static std::pair<Image, Image> gradient(Image const &img, unsigned gauss = 0) {
  Image src = img.clone();
  cv::cvtColor(src, src, CV_BGR2GRAY);
  std::pair<Image, Image> grad;
  int ddepth = CV_64F;

  Scharr(src, grad.first, ddepth, 1, 0);
  Scharr(src, grad.second, ddepth, 0, 1);

  for (int k = 0; k < gauss; ++k) {
    GaussianBlur(grad.first, grad.first, cv::Size(3, 3), 0);
    GaussianBlur(grad.second, grad.second, cv::Size(3, 3), 0);
  }

#ifdef PRINT_META
  cv::imwrite("/Users/ivan/.supp/code/snakes/grad_x.jpg", grad.first);
  cv::imwrite("/Users/ivan/.supp/code/snakes/grad_y.jpg", grad.second);
#endif
  return grad;
}

static Image hessian(Image const &img, unsigned gauss = 0) {
  Image src = img.clone();
  cv::cvtColor(src, src, CV_BGR2GRAY);
  Image dXX, dYY, dXY, hessian;

  cv::Sobel(src, dXX, CV_64F, 2, 0);
  cv::Sobel(src, dYY, CV_64F, 0, 2);
  cv::Sobel(src, dXY, CV_64F, 1, 1);

  for (int k = 0; k < gauss; ++k) {
    GaussianBlur(dXX, dXX, cv::Size(3, 3), 0);
    GaussianBlur(dXY, dXY, cv::Size(3, 3), 0);
    GaussianBlur(dYY, dYY, cv::Size(3, 3), 0);
  }

  hessian = dXX.mul(dYY) - dXY.mul(dXY);

  hessian = invert_image(hessian);
  for (int k = 0; k < gauss; ++k) {
    GaussianBlur(hessian, hessian, cv::Size(3, 3), 0);
  }
#ifdef PRINT_META
  cv::imwrite("/Users/ivan/.supp/code/snakes/hess.jpg", hessian);
#endif
  return hessian;
}

void Snake::set_xs(std::vector<double> _xs) { xs = _xs; }

void Snake::set_ys(std::vector<double> _ys) { ys = _ys; }

void Snake::set_pentamat() {
  int nodes = (int)xs.size();
  double ds2 = atom * atom;
  double ds4 = ds2 * ds2;

  pentamat = cv::Mat::zeros(nodes, nodes, CV_64F);

  double a = tension * tick / ds2, b = stiffness * tick / ds4;
  double p = b, q = -a - 4 * b, r = 1 + 2 * a + 6 * b;
  std::vector<double> coeffs = { p, q, r, q, p };

  for (int k = 0; k < nodes; ++k) {
    for (int j = 0; j < 5; ++j) {
      pentamat.at<double>(k, (nodes + k + j - 2) % nodes) = coeffs[j];
    }
  }

  pentamat = pentamat.inv();
}

std::vector<double> Snake::get_xs() const { return xs; }

std::vector<double> Snake::get_ys() const { return ys; }

void Snake::update_raw_img() {
  cv::Mat raw_mat(img.size(), CV_8UC4);
  cv::cvtColor(img, raw_mat, CV_BGR2RGBA, 4);
  cv::imencode(".bmp", raw_mat, raw_img);
}

void *Snake::get_raw_img() {
  return raw_img.data();
}

std::size_t Snake::get_raw_img_size() {
  return raw_img_size;
}

bool Snake::is_closed() { return closed; }

int Snake::get_implicit() { return implicit; }

/**
 *  Initiates snake instance from json config
 */
Snake::Snake(std::string json_file_path) {
  std::ifstream json_file(json_file_path);
  std::string json_string((std::istreambuf_iterator<char>(json_file)),
                          std::istreambuf_iterator<char>());
  std::string err_text;

  auto json = json11::Json::parse(json_string, err_text);

  std::string vid_path = json["vid_path"].string_value();

  tension     = json["tension"].number_value();
  stiffness   = json["stiffness"].number_value();
  atom        = json["atom"].number_value();
  closed      = json["closed"].bool_value();
  implicit    = json["implicit"].int_value();
  line_weight = json["line_weight"].number_value();
  edge_weight = json["edge_weight"].number_value();
  term_weight = json["term_weight"].number_value();
  tick        = json["tick"].number_value();
  fixed       = json["fixed"].bool_value();
  threshold   = json["threshold"].number_value();
  
  vid = cv::VideoCapture(vid_path);
  
  if(!vid.isOpened()) {
    std::cout << "Could not open or find the video specified in config.\n";
    std::exit(0);
  }
  
  img = cv::Mat(vid.get(CV_CAP_PROP_FRAME_HEIGHT), vid.get(CV_CAP_PROP_FRAME_WIDTH), vid.get(CV_CAP_PROP_FORMAT));
  
  raw_img_size = img.total() * 4;
  raw_img = std::vector<uchar>(raw_img_size);
  
  if (!shift_frame(true)) {
    std::cout << "Could not read the frame.\n";
    std::exit(0);
  }
}

bool Snake::shift_frame(bool direction) {
  
  if (!direction)
  {
    int position = vid.get(CV_CAP_PROP_POS_FRAMES);
    if (position < 1)
    {
      return true;
    }
    vid.set(CV_CAP_PROP_POS_FRAMES, position - 1);
  }
  
  if (!vid.read(img)) {
    return false;
  }
  update_raw_img();
  
  grad = gradient(img, 1);
  hess = hessian(img, 1);
  
#ifdef PRINT_META
  save_double_heat_map(grad.first, "/Users/ivan/.supp/code/snakes/hgx.jpg");
  save_double_heat_map(grad.second, "/Users/ivan/.supp/code/snakes/hgy.jpg");
  save_double_heat_map(hess, "/Users/ivan/.supp/code/snakes/hh.jpg");
#endif
  abs_threshold(grad.first, threshold);
  abs_threshold(grad.second, threshold);
  abs_threshold(hess, threshold);
  
#ifdef PRINT_META
  save_double_heat_map(grad.first, "/Users/ivan/.supp/code/snakes/thgx.jpg");
  save_double_heat_map(grad.second, "/Users/ivan/.supp/code/snakes/thgy.jpg");
  save_double_heat_map(hess, "/Users/ivan/.supp/code/snakes/thh.jpg");
#endif
  return true;
}

/**
 *  Saves current snake state on an image to file
 *
 *  @param output_file_path path to desired output file
 */
void Snake::print_and_save(std::string const &output_file_path) {
  Image img_copy = img.clone();

  filled_circle(img_copy, xs[0], ys[0], 2);
  for (unsigned k = 1; k < xs.size(); ++k) {
    line(img_copy, xs[k - 1], ys[k - 1], xs[k], ys[k]);
    filled_circle(img_copy, xs[k], ys[k], 2);
  }
  cv::imwrite(output_file_path, img_copy);
}

/**
 *  Move snake to the next state
 */
bool Snake::update() {
  int nodes = (int)xs.size();

  if (is_closed()) {
    double x = (xs[0] + xs[nodes - 1]) / 2;
    double y = (ys[0] + ys[nodes - 1]) / 2;
    xs[0] = xs[nodes - 1] = x;
    ys[0] = ys[nodes - 1] = y;
  }

  cv::Mat x_dist(nodes, 1, CV_64F), y_dist(nodes, 1, CV_64F);

  for (int k = 0; k < nodes; ++k) {
    double x_force =
        grad.first.at<double>(ys[k], xs[k]) *
        (edge_weight * hess.at<double>(ys[k], xs[k]) - line_weight);
    double y_force =
        grad.second.at<double>(ys[k], xs[k]) *
        (edge_weight * hess.at<double>(ys[k], xs[k]) - line_weight);
    x_dist.at<double>(k, 0) = tick * x_force;
    y_dist.at<double>(k, 0) = tick * y_force;
  }

  cv::Mat forced_xs(xs), forced_ys(ys);

  forced_xs += x_dist;
  forced_ys += y_dist;
  
//  std::vector<double> prev_xs(xs), prev_ys(ys);
  
  xs = (cv::Mat)(pentamat * forced_xs);
  ys = (cv::Mat)(pentamat * forced_ys);
  
//  double acc_x = 0, acc_y = 0;
//  double eps = ???;
//  
//  for (int k = 0; k < nodes; ++k)
//  {
//    acc_x += xs[k] - prev_xs[k];
//    acc_y += ys[k] - prev_ys[k];
//  }
//  acc_x += acc_y; acc_x /= tick;
//  
//  if (std::abs(acc_x) < eps)
//  {
//    return true;
//  }
  
  return false;
}
