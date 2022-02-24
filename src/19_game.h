#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <tuple>

#include "19_sprite_renderer.h"
#include "19_resource_manager.h"
#include "19_gamelevel.h"
#include "19_ball_object.h"
#include "19_particle_generator.h"

//게임 state
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};
//방향
enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

//player paddle 설정
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f); //크기
const float PLAYER_VELOCITY(500.0f);  //속도

//ball 설정
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f); //속도
//const glm::vec2 INITIAL_BALL_VELOCITY(10.0f, -35.0f);
const float BALL_RADIUS = 12.5f; //크기, 반지름

//충돌데이터 튜플
typedef std::tuple<bool, Direction, glm::vec2> Collision;

//충돌 방향 벡터 구하기
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
} 
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
Collision CheckCollision(BallObject &one, GameObject &two) // AABB-Circle
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
    if (glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}     

//게임 클래스
class Game
{
private:
    SpriteRenderer *Renderer;
    GameObject *Player;
    BallObject *Ball;
    ParticleGenerator *Particles;

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
        delete Ball;
        delete Particles;
    }

    // initialize game state (load all shaders/textures/levels)
    void Init()
    {
        // load shaders
        ResourceManager::LoadShader("src/shaders/19sprite.vs", "src/shaders/19sprite.fs", nullptr, "sprite");
        ResourceManager::LoadShader("src/shaders/19particle.vs", "src/shaders/19particle.fs", nullptr, "particle");
        // configure shaders
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
            static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
        ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
        ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
        ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
        ResourceManager::GetShader("particle").SetMatrix4("projection", projection); 
        // load textures
        ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
        ResourceManager::LoadTexture("textures/block.png", false, "block");
        ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
        ResourceManager::LoadTexture("textures/background.jpg", false, "background");
        ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
        ResourceManager::LoadTexture("textures/particle.png", true, "particle");
        // set render-specific controls
        Shader renderershader = ResourceManager::GetShader("sprite");
        Renderer = new SpriteRenderer(renderershader);
        Shader particleshader = ResourceManager::GetShader("particle");
        Particles = new ParticleGenerator(particleshader, ResourceManager::GetTexture("particle"), 500);
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
    // game reset
    void ResetLevel()
    {
        if (this->Level == 0)
            this->Levels[0].Load("resources/gamelevel/1.txt", this->Width, this->Height / 2);
        else if (this->Level == 1)
            this->Levels[1].Load("resources/gamelevel/2.txt", this->Width, this->Height / 2);
        else if (this->Level == 2)
            this->Levels[2].Load("resources/gamelevel/3.txt", this->Width, this->Height / 2);
        else if (this->Level == 3)
            this->Levels[3].Load("resources/gamelevel/4.txt", this->Width, this->Height / 2);
    }
    // player reset
    void ResetPlayer()
    {
        // reset player/ball stats
        Player->Size = PLAYER_SIZE;
        Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
        Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}
    // next level
    void NextLevel()
    {
        this->Level++;
        ResetLevel();
        ResetPlayer();
    }
    // 충돌 처리
    void DoCollisions()
    {
        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if (!box.Destroyed)
            {
                Collision collision = CheckCollision(*Ball, box);
                if (std::get<0>(collision)) // if collision is true
                {
                    // destroy block if not solid
                    if (!box.IsSolid)
                        box.Destroyed = true;
                    // collision resolution
                    Direction dir = std::get<1>(collision);
                    glm::vec2 diff_vector = std::get<2>(collision);
                    if (dir == LEFT || dir == RIGHT) // horizontal collision
                    {
                        Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration; // move ball to right
                        else
                            Ball->Position.x -= penetration; // move ball to left;
                    }
                    else // vertical collision
                    {
                        Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration; // move ball back up
                        else
                            Ball->Position.y += penetration; // move ball back down
                    }
                }
            }
        }
        Collision result = CheckCollision(*Ball, *Player);
        if (!Ball->Stuck && std::get<0>(result))
        {
            // check where it hit the board, and change velocity based on where it hit the board
            float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
            float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
            float percentage = distance / (Player->Size.x / 2.0f);
            // then move accordingly
            float strength = 2.0f;
            glm::vec2 oldVelocity = Ball->Velocity;
            Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
            Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);  
            Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        } 
    }   

    //게임 상황 업데이트
    void Update(float dt)
    {
        // 오브젝트 업데이트
        Ball->Move(dt, this->Width);
        // 충돌 감지
        this->DoCollisions();
        // 파티클 업데이트
        Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
        // 게임 오버
        if (Ball->Position.y >= this->Height)
        {
            this->ResetLevel();
            this->ResetPlayer();
        }
        // 레벨 클리어, 다음 레벨 진행
        if(this->Levels[this->Level].IsCompleted())
            NextLevel();
    }
    //게임 렌더링
    void Render()
    {
        if (this->State == GAME_ACTIVE)
        {
            // draw background
            Texture2D background = ResourceManager::GetTexture("background");
            Renderer->DrawSprite(background, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
            // draw level
            this->Levels[this->Level].Draw(*Renderer);
            // draw player
            Player->Draw(*Renderer);
            // draw particlse
            Particles->Draw();
            // draw ball
            Ball->Draw(*Renderer);
        }
    }
};

#endif
