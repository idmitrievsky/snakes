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

static void line(Image &img, int start_x, int start_y, int end_x, int end_y)
{
    int thickness = 1, line_type = 8;
    cv::Point start = cv::Point(start_x, start_y),
              end   = cv::Point(end_x, end_y);
    
    cv::line(img, start, end, cv::Scalar(255, 255, 255), thickness, line_type);
}

static void filled_circle(Image &img, int center_x, int center_y)
{
    int thickness = 1, line_type = 8;
    cv::Point center = cv::Point(center_x, center_y);
    
    cv::circle(img, center, 2, cv::Scalar(255, 255, 255), thickness, line_type);
}

static std::pair<Image, Image> gradient(Image const &img, bool gauss = false, std::pair<Image, Image> *hess = 0)
{
    Image src = img.clone();
    std::pair<Image, Image> grad;
    
    int ddepth = CV_64F;
    
    if (gauss)
    {
        GaussianBlur(src, src, cv::Size(3, 3), 0);
    }

    Scharr(src, grad.first, ddepth, 1, 0);
    Scharr(src, grad.second, ddepth, 0, 1);
    
    if (hess)
    {
        Scharr(grad.first, hess->first, ddepth, 1, 0);
        Scharr(grad.second, hess->second, ddepth, 0, 1);
    }
    
    return grad;
}

Snake::Snake(std::string json_file_path)
{
    std::ifstream json_file(json_file_path);
    std::string json_string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
    std::string err_text;
    
    auto json = json11::Json::parse(json_string, err_text);
    
    img = cv::imread(json["img_path"].string_value(), CV_LOAD_IMAGE_GRAYSCALE);
    
    tension = json["tension"].number_value();
    stiffness = json["stiffness"].number_value();
    line_weight = json["line_weight"].number_value();
    edge_weight = json["edge_weight"].number_value();
    term_weight = json["term_weight"].number_value();
    
    bool radial_nodes_init = json.has_shape({{"center", json11::Json::ARRAY}}, err_text);
    
    if (radial_nodes_init)
    {
        int nodes_num = json["nodes_num"].int_value();
        double step = 2 * 3.1415 / (nodes_num - 1),
        center_x = json["center"][0].number_value(),
        center_y = json["center"][1].number_value(),
        radius = json["radius"].number_value();
        
        for (int k = 0; k < nodes_num; ++k)
        {
            xs.push_back(center_x + radius * std::cos(step * k));
            ys.push_back(center_y + radius * std::sin(step * k));
        }
    }
    else
        for (auto &node: json["nodes"].array_items())
        {
            xs.push_back(node[0].number_value());
            ys.push_back(node[1].number_value());
        }
}

void Snake::print_and_save(std::string const &output_file_path)
{
    Image img_copy = img.clone();
    
    filled_circle(img_copy, xs[0], ys[0]);
    for (unsigned k = 1; k < xs.size(); ++k)
    {
        line(img_copy, xs[k - 1], ys[k - 1], xs[k], ys[k]);
        filled_circle(img_copy, xs[k], ys[k]);
    }
    cv::imwrite(output_file_path, img_copy);
}

void Snake::move()
{
    int nodes = (int)xs.size();
    
    std::pair<Image, Image> hess;
    arma::mat penta = arma::zeros<arma::mat>(nodes, nodes);
    std::pair<Image, Image> grad = gradient(img, false, &hess);
    
    double ds = 1.0, ds2 = ds * ds, dt = 0.05;
    
    for (int k = 0; k < nodes; ++k)
    {
        xs[k] += dt * (grad.first.at<double>(xs[k], ys[k]) * (edge_weight * hess.first.at<double>(xs[k], ys[k]) - line_weight));
        ys[k] += dt * (grad.second.at<double>(xs[k], ys[k]) * (edge_weight * hess.second.at<double>(xs[k], ys[k]) - line_weight));
    }
    
    double a = tension * dt / ds2, b = stiffness * dt / ds2;
    double p = b, q = -a - 4 * b, r = 1 + 2 * a + 6 * b;
    std::vector<double> coeffs = {p, q, r, q, p};
    
    for (int k = 0; k < nodes; ++k)
    {
        for (int j = 0; j < 5; ++j)
        {
            penta(k, (nodes + k + j - 2) % nodes) = coeffs[j];
        }
    }
    
    arma::vec _xs(xs), _ys(ys);
    arma::vec new_xs(nodes), new_ys(nodes);
    
    arma::solve(new_xs, penta, _xs);
    arma::solve(new_ys, penta, _ys);
    
    for (int k = 0; k < nodes - 1; ++k)
    {
        xs[k] = new_xs[k];
        ys[k] = new_ys[k];
    }
    xs[nodes - 1] = new_xs[0];
    ys[nodes - 1] = new_ys[0];
}