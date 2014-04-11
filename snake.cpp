//
//  snake.cpp
//  snakes
//
//  Created by Ivan Dmitrievsky on 11/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#include "snake.h"

static void line(Image &img, Pnt start, Pnt end)
{
    int thickness = 1, line_type = 8;
    cv::Point _start = cv::Point(start(0), start(1)),
              _end   = cv::Point(end(0), end(1));
    
    cv::line(img, _start, _end, cv::Scalar(0, 0, 255), thickness, line_type);
}

static void filled_circle(Image &img, Pnt center)
{
    int thickness = 1, line_type = 8;
    cv::Point _center = cv::Point(center(0), center(1));
    
    cv::circle(img, _center, 2, cv::Scalar(0, 0, 255), thickness, line_type);
}

Snake::Snake(PntSeq _nodes)
{
    nodes = _nodes;
}

void Snake::print_and_save(Image img, std::string const &output_file_path)
{
    filled_circle(img, nodes[0]);
    for (unsigned k = 1; k < nodes.size(); ++k)
    {
        line(img, nodes[k - 1], nodes[k]);
        filled_circle(img, nodes[k]);
    }
    cv::imwrite(output_file_path, img);
}