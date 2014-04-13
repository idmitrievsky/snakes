//
//  main.cpp
//  snakes
//
//  Created by Ivan Dmitrievsky on 11/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#include "json11.hpp"
#include "snake.h"

int main(int argc, const char * argv[])
{
    std::string snake_config("/Users/ivan/.supp/code/snakes/snake.json");
    
    Snake snake(snake_config);
    
    snake.print_and_save("/Users/ivan/.supp/code/snakes/start.png");
    for (int k = 0; k < 40; ++k)
    {
        snake.move();
    }
    snake.print_and_save("/Users/ivan/.supp/code/snakes/end.png");
    
    return 0;
}

