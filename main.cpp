//
//  main.cpp
//  snakes
//
//  Created by Ivan Dmitrievsky on 11/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#include "snake.h"

int main(int argc, const char * argv[])
{
    cv::Mat img = cv::imread("/Users/ivan/.supp/code/snakes/sample.png", CV_LOAD_IMAGE_ANYDEPTH);
    
    auto snake = Snake({{20, 20}, {30, 30}, {40, 30}, {50, 40}, {90, 10}});
    
    snake.print_and_save(img, "/Users/ivan/.supp/code/snakes/out.png");
    
    return 0;
}

