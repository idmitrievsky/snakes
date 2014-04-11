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
#include <vector>
#include <string>
#define  ARMA_DONT_USE_WRAPPER
#include "armadillo"

typedef arma::vec2 Pnt;
typedef std::vector<Pnt> PntSeq;
typedef cv::Mat Image;

class Snake
{
    PntSeq nodes;
public:
    Snake(PntSeq _nodes);
    void print_and_save(Image img, std::string const &output_file_path);
};

#endif /* defined(__snakes__snake__) */
