#ifndef BALLOBJECT_H
#define BALLOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "19_game_object.h"
#include "19_texture.h"


// BallObject holds the state of the Ball object inheriting
// relevant state data from GameObject. Contains some extra
// functionality specific to Breakout's ball object that
// were too specific for within GameObject alone.
class BallObject : public GameObject
{
public:
    // ball state	
    float   Radius;
    bool    Stuck;
    bool    Sticky, PassThrough;

    // constructor(s)
    BallObject() : GameObject(), Radius(12.5f), Stuck(true), Sticky(false), PassThrough(false) { }
    BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite)
        : GameObject(pos, glm::vec2(radius * 2.0f, radius * 2.0f), sprite, glm::vec3(1.0f), velocity), Radius(radius), Stuck(true), Sticky(false), PassThrough(false) { }

    // moves the ball, keeping it constrained within the window bounds (except bottom edge); returns new position
    glm::vec2 Move(float dt, unsigned int window_width)
    {
        // if not stuck to player board
        if (!this->Stuck)
        {
            // move the ball
            this->Position += this->Velocity * dt;
            // then check if outside window bounds and if so, reverse velocity and restore at correct position
            if (this->Position.x <= 0.0f)
            {
                this->Velocity.x = -this->Velocity.x;
                this->Position.x = 0.0f;
            }
            else if (this->Position.x + this->Size.x >= window_width)
            {
                this->Velocity.x = -this->Velocity.x;
                this->Position.x = window_width - this->Size.x;
            }
            if (this->Position.y <= 0.0f)
            {
                this->Velocity.y = -this->Velocity.y;
                this->Position.y = 0.0f;
            }
        }
        return this->Position;
    }
    // resets the ball to original state with given position and velocity
    void Reset(glm::vec2 position, glm::vec2 velocity)
    {
        this->Position = position;
        this->Velocity = velocity;
        this->Stuck = true;
        this->Sticky = false;
        this->PassThrough = false;
    }
};

#endif