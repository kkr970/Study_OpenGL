#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "19_sprite_renderer.h"
#include "19_resource_manager.h"
#include "19_gamelevel.h"
#include "19_ball_object.h"

//게임 state
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

//player paddle 설정
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
const float PLAYER_VELOCITY(500.0f);

//ball 설정
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
//const glm::vec2 INITIAL_BALL_VELOCITY(10.0f, -35.0f);
const float BALL_RADIUS = 12.5f;

//충돌 함수
bool CheckCollision(GameObject &one, GameObject &two) //AABB-AABB
{
    //x축
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    //y축
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    //x, y둘다 감지되면 충돌
    return collisionX && collisionY;
} 
bool CheckCollision(BallObject &one, GameObject &two) // AABB-Circle
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
        two.Position.x + aabb_half_extents.x, 
        two.Position.y + aabb_half_extents.y
    );
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;
    return glm::length(difference) < one.Radius;
}     

//게임 클래스
class Game
{
private:
    //스프라이트 렌더러
    SpriteRenderer *Renderer;
    GameObject *Player;
    BallObject *Ball;

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
            // player paddle
        glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
            // ball
        glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
        Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
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
                {
                    Player->Position.x -= velocity;
                    if (Ball->Stuck)
                        Ball->Position.x -= velocity;
                }
            }
            if (this->Keys[GLFW_KEY_D])
            {
                if (Player->Position.x <= this->Width - Player->Size.x)
                {
                    Player->Position.x += velocity;
                    if (Ball->Stuck)
                        Ball->Position.x += velocity;
                }
            }
            // ball stuck
            if (this->Keys[GLFW_KEY_SPACE])
                Ball->Stuck = false;
        }
    }

    void DoCollisions()
    {
        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if (!box.Destroyed)
            {
                if (CheckCollision(*Ball, box))
                {
                    if (!box.IsSolid)
                        box.Destroyed = true;
                }
            }
        }
    } 

    void Update(float dt)
    {
        // 오브젝트 업데이트
        Ball->Move(dt, this->Width);

        // 충돌 감지
        this->DoCollisions();
    }

    void Render()
    {
        if(this->State == GAME_ACTIVE)
        {
            if (this->State == GAME_ACTIVE)
            {
                // draw background
                Texture2D background = ResourceManager::GetTexture("background");
                Renderer->DrawSprite(background, glm::vec2(0.0f, 0.0f),
                                    glm::vec2(this->Width, this->Height), 0.0f);
                // draw level
                this->Levels[this->Level].Draw(*Renderer);
                // draw player
                Player->Draw(*Renderer);
                // draw ball
                Ball->Draw(*Renderer);
            }
        }
    }
};

#endif
