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
#define  ARMA_DONT_USE_WRAPPER
#include "armadillo"


typedef arma::ivec2 Point;
typedef cv::Mat Image;

class Snake
{
    double tension;
    double stiffness;
    
    double line_weight;
    double edge_weight;
    double term_weight;
    
    cv::Mat img;
    
    std::vector<double> xs;
    std::vector<double> ys;
public:
    Snake(arma::vec6 snake_description);
    Snake(arma::vec7 snake_description);
    Snake(std::string json_file_path);
    void move();
    void print_and_save(std::string const &output_file_path);
};

#endif /* defined(__snakes__snake__) */
