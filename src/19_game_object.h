#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "19_texture.h"
#include "19_sprite_renderer.h"


// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObject
{
public:
    // object state
    glm::vec2   Position, Size, Velocity;
    glm::vec3   Color;
    float       Rotation;
    bool        IsSolid;
    bool        Destroyed;
    // render state
    Texture2D   Sprite;	
    // constructor(s)
    GameObject()
            : Position(0.0f, 0.0f), Size(1.0f, 1.0f), Velocity(0.0f),
            Color(1.0f), Rotation(0.0f), Sprite(), IsSolid(false), Destroyed(false) { }
    GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f), glm::vec2 velocity = glm::vec2(0.0f, 0.0f))
            : Position(pos), Size(size), Velocity(velocity),
            Color(color), Rotation(0.0f), Sprite(sprite), IsSolid(false), Destroyed(false) { }
    // draw sprite
    virtual void Draw(SpriteRenderer &renderer)
    {
        renderer.DrawSprite(this->Sprite, this->Position, this->Size, this->Rotation, this->Color);
    }
};

#endif
