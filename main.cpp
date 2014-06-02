//
//  main.cpp
//  snakes
//
//  Created by Ivan Dmitrievsky on 11/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#include "sfml.h"
#include "json11.hpp"
#include "snake.h"

int main(int argc, const char* argv[]) {
  std::string snake_config("/Users/ivan/.supp/code/snakes/snake.json");

  Snake snake(snake_config);

  sfml_loop(snake);

  return 0;
}
