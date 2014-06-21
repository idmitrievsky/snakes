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
#include <streambuf>
#include "json11.hpp"
#include "snake.h"

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
 *  @param gauss           blur gradient or not
 *  @param hess            calculate hessian or not
 *
 *  @return pair of images, each representing one of gradient components
 */
static std::pair<Image, Image> gradient(Image const &img, bool gauss = false,
                                        std::pair<Image, Image> *hess = 0) {
  Image src = img.clone();
  cv::cvtColor(src, src, CV_BGR2GRAY);
  std::pair<Image, Image> grad;

  int ddepth = CV_64F;

  Scharr(src, grad.first, ddepth, 1, 0);
  Scharr(src, grad.second, ddepth, 0, 1);

  if (gauss)
    for (int k = 0; k < 2; ++k) {
      GaussianBlur(grad.first, grad.first, cv::Size(3, 3), 0);
      GaussianBlur(grad.second, grad.second, cv::Size(3, 3), 0);
    }

  cv::imwrite("/Users/ivan/.supp/code/snakes/grad_x.jpg", grad.first);
  cv::imwrite("/Users/ivan/.supp/code/snakes/grad_y.jpg", grad.second);

  if (hess) {
    Scharr(grad.first, hess->first, ddepth, 1, 0);
    Scharr(grad.second, hess->second, ddepth, 0, 1);

    if (gauss)
      for (int k = 0; k < 2; ++k) {
        GaussianBlur(hess->first, hess->first, cv::Size(3, 3), 0);
        GaussianBlur(hess->second, hess->second, cv::Size(3, 3), 0);
      }

    cv::imwrite("/Users/ivan/.supp/code/snakes/hess_x.jpg", hess->first);
    cv::imwrite("/Users/ivan/.supp/code/snakes/hess_y.jpg", hess->second);
  }

  return grad;
}

std::string Snake::image_path() { return img_path; }

void Snake::set_xs(std::vector<double> _xs) { xs = _xs; }

void Snake::set_ys(std::vector<double> _ys) { ys = _ys; }

std::vector<double> Snake::get_xs() const { return xs; }

std::vector<double> Snake::get_ys() const { return ys; }

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

  img_path = json["img_path"].string_value();
  img = cv::imread(img_path);

  grad = gradient(img, true, &hess);

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
void Snake::update() {
  int nodes = (int)xs.size();

  arma::mat penta = arma::zeros<arma::mat>(nodes, nodes);

  double ds2 = atom * atom;

  if (is_closed()) {
    double x = (xs[0] + xs[nodes - 1]) / 2;
    double y = (ys[0] + ys[nodes - 1]) / 2;
    xs[0] = xs[nodes - 1] = x;
    ys[0] = ys[nodes - 1] = y;
  }

  arma::vec x_force(nodes), y_force(nodes);

  for (int k = 0; k < nodes; ++k) {
    x_force(k) =
        tick *
        (grad.first.at<double>(xs[k], ys[k]) *
         (edge_weight * hess.first.at<double>(xs[k], ys[k]) - line_weight));
    y_force(k) =
        tick *
        (grad.second.at<double>(xs[k], ys[k]) *
         (edge_weight * hess.second.at<double>(xs[k], ys[k]) - line_weight));
  }

  double a = tension * tick / ds2, b = stiffness * tick / ds2;
  double p = b, q = -a - 4 * b, r = 1 + 2 * a + 6 * b;
  std::vector<double> coeffs = { p, q, r, q, p };

  for (int k = 0; k < nodes; ++k)
    for (int j = 0; j < 5; ++j)
      penta(k, (nodes + k + j - 2) % nodes) = coeffs[j];

  arma::vec _xs(xs), _ys(ys);
  arma::vec new_xs(nodes), new_ys(nodes);

  arma::solve(new_xs, penta, _xs + x_force);
  arma::solve(new_ys, penta, _ys + y_force);

  for (int k = fixed; k < nodes - fixed; ++k) {
    xs[k] = new_xs[k];
    ys[k] = new_ys[k];
  }
}