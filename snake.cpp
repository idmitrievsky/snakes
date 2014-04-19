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
    
    cv::line(img, start, end, cv::Scalar(0, 0, 255), thickness, line_type);
}

static void filled_circle(Image &img, int center_x, int center_y)
{
    int thickness = 1, line_type = 8;
    cv::Point center = cv::Point(center_x, center_y);
    
    cv::circle(img, center, 2, cv::Scalar(0, 0, 255), thickness, line_type);
}

static std::pair<Image, Image> gradient(Image const &img, bool gauss = false, std::pair<Image, Image> *hess = 0)
{
    Image src = img.clone();
    cv::cvtColor(src, src, CV_BGR2GRAY);
    std::pair<Image, Image> grad;
    
    int ddepth = CV_64F;

//    Sobel(src, grad.first, ddepth, 1, 0, 3);
//    Sobel(src, grad.second, ddepth, 0, 1, 3);
    
    Scharr(src, grad.first, ddepth, 1, 0);
    Scharr(src, grad.second, ddepth, 0, 1);
    
    if (gauss)
        for (int k = 0; k < 2; ++k)
        {
            GaussianBlur(grad.first, grad.first, cv::Size(3, 3), 0);
            GaussianBlur(grad.second, grad.second, cv::Size(3, 3), 0);
        }
    
    cv::imwrite("/Users/ivan/.supp/code/snakes/grad_x.jpg", grad.first);
    cv::imwrite("/Users/ivan/.supp/code/snakes/grad_y.jpg", grad.second);
    
    if (hess)
    {
//        Sobel(grad.first, hess->first, ddepth, 1, 0, 3);
//        Sobel(grad.second, hess->second, ddepth, 0, 1, 3);
        
        Scharr(grad.first, hess->first, ddepth, 1, 0);
        Scharr(grad.second, hess->second, ddepth, 0, 1);
        
        if (gauss)
            for (int k = 0; k < 2; ++k)
            {
                GaussianBlur(hess->first, hess->first, cv::Size(3, 3), 0);
                GaussianBlur(hess->second, hess->second, cv::Size(3, 3), 0);
            }
        
        cv::imwrite("/Users/ivan/.supp/code/snakes/hess_x.jpg", hess->first);
        cv::imwrite("/Users/ivan/.supp/code/snakes/hess_y.jpg", hess->second);
    }
    
    return grad;
}

std::string Snake::image_path()
{
    return img_path;
}

void Snake::set_xs(std::vector<double> _xs)
{
    xs = _xs;
}

void Snake::set_ys(std::vector<double> _ys)
{
    ys = _ys;
}

std::vector<double> Snake::get_xs() const
{
    return xs;
}

std::vector<double> Snake::get_ys() const
{
    return ys;
}

bool Snake::is_closed()
{
    return closed;
}

Snake::Snake(std::string json_file_path)
{
    std::ifstream json_file(json_file_path);
    std::string json_string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
    std::string err_text;
    
    auto json = json11::Json::parse(json_string, err_text);
    
    img_path = json["img_path"].string_value();
    img = cv::imread(img_path);
    
    grad = gradient(img, true, &hess);
    
    tension     = json["tension"].number_value();
    stiffness   = json["stiffness"].number_value();
    line_weight = json["line_weight"].number_value();
    edge_weight = json["edge_weight"].number_value();
    term_weight = json["term_weight"].number_value();
    fixed       = json["fixed"].bool_value();
    closed      = json["closed"].bool_value();
    atom        = json["atom"].number_value();
    tick        = json["tick"].number_value();
    
    bool radial_nodes_init = json.has_shape({{"center", json11::Json::ARRAY}}, err_text);
    
    if (radial_nodes_init)
    {
        int nodes_num = json["nodes_num"].int_value();
        double step   = 2 * 3.1415 / (nodes_num - 1),
        center_x      = json["center"][0].number_value(),
        center_y      = json["center"][1].number_value(),
        radius        = json["radius"].number_value();
        
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

void Snake::update()
{
    int nodes = (int)xs.size();
    
    arma::mat penta = arma::zeros<arma::mat>(nodes, nodes);

    double ds2 = atom * atom;
    
    
    for (int k = 0; k < nodes; ++k)
    {
        x_force(k) = tick * (grad.first.at<double>(xs[k], ys[k]) * (edge_weight * hess.first.at<double>(xs[k], ys[k]) - line_weight));
        y_force(k) = tick * (grad.second.at<double>(xs[k], ys[k]) * (edge_weight * hess.second.at<double>(xs[k], ys[k]) - line_weight));
    }
    
    double a = tension * tick / ds2, b = stiffness * tick / ds2;
    double p = b, q = -a - 4 * b, r = 1 + 2 * a + 6 * b;
    std::vector<double> coeffs = {p, q, r, q, p};
    
    for (int k = 0; k < nodes; ++k)
        for (int j = 0; j < 5; ++j)
            penta(k, (nodes + k + j - 2) % nodes) = coeffs[j];
    
    arma::vec _xs(xs), _ys(ys);
    arma::vec new_xs(nodes), new_ys(nodes);
    
    arma::solve(new_xs, penta, _xs + x_force);
    arma::solve(new_ys, penta, _ys + y_force);
    
    for (int k = fixed; k < nodes - fixed; ++k)
    {
        xs[k] = new_xs[k];
        ys[k] = new_ys[k];
    }
    xs[nodes - 1] = new_xs[0];
    ys[nodes - 1] = new_ys[0];
}