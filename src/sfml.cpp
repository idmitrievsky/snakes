//
//  sfml.cpp
//  snakes
//
//  Created by Ivan Dmitrievsky on 19/04/14.
//  Copyright (c) 2014 Ivan Dmitrievsky. All rights reserved.
//

#include "sfml.h"

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

void sfml_loop(Snake &snake)
{
    sf::Image image;
    image.loadFromFile(snake.image_path());
    
    sf::RenderWindow window({image.getSize().x, image.getSize().y}, snake.image_path(), sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite;
    sprite.setTexture(texture);
    
    unsigned long snake_size = 0;
    std::vector<double> xs(snake_size), ys(snake_size);
    
    bool inited =  false, play = false;
    std::vector<sf::Vertex> snake_scheme;
    snake_scheme.push_back(sf::Vertex({0,0}, sf::Color::Red));
    
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::MouseMoved)
                if (!inited)
                    snake_scheme.back().position = sf::Vector2f(sf::Mouse::getPosition(window));
            
            if (event.type == sf::Event::MouseButtonPressed)
                if (!inited)
                    snake_scheme.insert(snake_scheme.end() - 1, sf::Vertex(sf::Vector2f(sf::Mouse::getPosition(window)), sf::Color::Red));
            
            if (event.type == sf::Event::KeyPressed && snake_scheme.size() > 2)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (event.key.code == sf::Keyboard::Space)
                    play = !play;
                
                if (!inited)
                {
                    if (snake.is_closed())
                        snake_scheme.back() = snake_scheme.front();
                    else
                        snake_scheme.pop_back();
                    
                    snake_size = snake.get_implicit() * (snake_scheme.size() - 1) + 1;
                    xs.resize(snake_size); ys.resize(snake_size);
                    
                    for (int k = 0; k < snake_scheme.size() - 1; ++k)
                    {
                        auto x_diff = (snake_scheme[k + 1].position.x - snake_scheme[k].position.x) / snake.get_implicit();
                        auto y_diff = (snake_scheme[k + 1].position.y - snake_scheme[k].position.y) / snake.get_implicit();
                        for (int j = 0; j < snake.get_implicit(); ++j)
                        {
                            xs[k * snake.get_implicit() + j] = snake_scheme[k].position.x + j * x_diff;
                            ys[k * snake.get_implicit() + j] = snake_scheme[k].position.y + j * y_diff;
                        }
                    }
                    xs.back() = snake_scheme[snake_scheme.size() - 1].position.x;
                    ys.back() = snake_scheme[snake_scheme.size() - 1].position.y;
                    snake_scheme.resize(snake_size, {{0, 0}, sf::Color::Red});
                    snake.set_xs(xs);
                    snake.set_ys(ys);
                }
                inited = true;
            }
            
        }
        
        if (inited && play)
        {
            snake.update();
            xs = snake.get_xs(); ys = snake.get_ys();
            for (int k = 0; k < snake_size; ++k)
            {
                snake_scheme[k].position.x = xs[k];
                snake_scheme[k].position.y = ys[k];
            }
        }
        
        window.draw(sprite);
        window.draw(&snake_scheme[0], (unsigned)snake_scheme.size(), sf::LinesStrip);
        
        window.display();
    }

}