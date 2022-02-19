#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "19_sprite_renderer.h"
#include "19_resource_manager.h"

//게임 state
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

//게임 클래스
class Game
{
private:
    //스프라이트 렌더러
    SpriteRenderer *Renderer;

public:
    // game state
    GameState State;	
    bool Keys[1024];
    unsigned int Width, Height;

    // constructor/destructor
    Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
    { 

    }
    ~Game()
    {
        delete Renderer;
    }

    // initialize game state (load all shaders/textures/levels)
    void Init()
    {
        // load shaders
        ResourceManager::LoadShader("src/shaders/19sprite.vs", "src/shaders/19sprite.fs", nullptr, "sprite");
        // configure shaders
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
            static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
        ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
        ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
        // set render-specific controls
        Shader shader = ResourceManager::GetShader("sprite");
        Renderer = new SpriteRenderer(shader);
        // load textures
        ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    }
    // game loop
    void ProcessInput(float dt)
    {

    }
    void Update(float dt)
    {

    }
    void Render()
    {
        Texture2D texture = ResourceManager::GetTexture("face");
        Renderer->DrawSprite(texture, glm::vec2(200.0f, 200.0f),
                            glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

#endif
