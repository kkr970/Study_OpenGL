#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "19_sprite_renderer.h"
#include "19_resource_manager.h"
#include "19_gamelevel.h"

//게임 state
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

//player paddle 설정
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
const float PLAYER_VELOCITY(500.0f);

//게임 클래스
class Game
{
private:
    //스프라이트 렌더러
    SpriteRenderer *Renderer;
    GameObject *Player;

public:
    // game state
    GameState State;	
    bool Keys[1024];
    unsigned int Width, Height;
    std::vector<GameLevel> Levels;
    unsigned int Level;

    // constructor/destructor
    Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
    { 

    }
    ~Game()
    {
        delete Renderer;
        delete Player;
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
        ResourceManager::LoadTexture("textures/block.png", false, "block");
        ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
        ResourceManager::LoadTexture("textures/background.jpg", false, "background");
        ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
        // load levels
        GameLevel one; one.Load("resources/gamelevel/1.txt", this->Width, this->Height / 2);
        GameLevel two; two.Load("resources/gamelevel/2.txt", this->Width, this->Height / 2);
        GameLevel three; three.Load("resources/gamelevel/3.txt", this->Width, this->Height / 2);
        GameLevel four; four.Load("resources/gamelevel/4.txt", this->Width, this->Height / 2);
        this->Levels.push_back(one);
        this->Levels.push_back(two);
        this->Levels.push_back(three);
        this->Levels.push_back(four);
        this->Level = 0;
        // configure game objects
        glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    }
    // game loop
    void ProcessInput(float dt)
    {
        if (this->State == GAME_ACTIVE)
        {
            float velocity = PLAYER_VELOCITY * dt;
            // move playerboard
            if (this->Keys[GLFW_KEY_A])
            {
                if (Player->Position.x >= 0.0f)
                    Player->Position.x -= velocity;
            }
            if (this->Keys[GLFW_KEY_D])
            {
                if (Player->Position.x <= this->Width - Player->Size.x)
                    Player->Position.x += velocity;
            }
        }
    }

    void Update(float dt)
    {

    }

    void Render()
    {
        if(this->State == GAME_ACTIVE)
        {
            // draw background
            Texture2D background = ResourceManager::GetTexture("background");
            Renderer->DrawSprite(background, glm::vec2(0.0f, 0.0f),
                                glm::vec2(this->Width, this->Height), 0.0f);
            // draw level
            this->Levels[this->Level].Draw(*Renderer);
            // draw player
            Player->Draw(*Renderer);
        }
    }
};

#endif
