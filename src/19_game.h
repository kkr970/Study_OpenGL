#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <tuple>
#include <algorithm>

#include "19_sprite_renderer.h"
#include "19_resource_manager.h"
#include "19_gamelevel.h"
#include "19_ball_object.h"
#include "19_particle_generator.h"
#include "19_post_processor.h"
#include "19_power_ups.h"

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

//후처리 효과 변수
float ShakeTime = 0.0f;

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

// 파워업 생성 확률
bool ShouldSpawn(unsigned int chance)
{
    unsigned int random = rand() % chance;
    return random == 0;
}

// powerup 활성화 상태 확인
bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
{
    // Check if another PowerUp of the same type is still active
    // in which case we don't disable its effect (yet)
    for (const PowerUp &powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}

//게임 클래스-------------------------------------------------------------------------------------------
class Game
{
private:
    SpriteRenderer *Renderer;
    GameObject *Player;
    BallObject *Ball;
    ParticleGenerator *Particles;
    PostProcessor *Effects;

public:
    // game state
    GameState State;	
    bool Keys[1024];
    unsigned int Width, Height;
    std::vector<GameLevel> Levels;
    std::vector<PowerUp> PowerUps;
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
        delete Effects;
    }

    // initialize game state (load all shaders/textures/levels)
    void Init()
    {
        // load shaders
        ResourceManager::LoadShader("src/shaders/19sprite.vs", "src/shaders/19sprite.fs", nullptr, "sprite");
        ResourceManager::LoadShader("src/shaders/19particle.vs", "src/shaders/19particle.fs", nullptr, "particle");
        ResourceManager::LoadShader("src/shaders/19postprocessing.vs", "src/shaders/19postprocessing.fs", nullptr, "postprocessing");
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
        ResourceManager::LoadTexture("textures/powerups/powerup_chaos.png", true, "powerup_chaos");
        ResourceManager::LoadTexture("textures/powerups/powerup_confuse.png", true, "powerup_confuse");
        ResourceManager::LoadTexture("textures/powerups/powerup_increase.png", true, "powerup_increase");
        ResourceManager::LoadTexture("textures/powerups/powerup_passthrough.png", true, "powerup_passthrough");
        ResourceManager::LoadTexture("textures/powerups/powerup_speed.png", true, "powerup_speed");
        ResourceManager::LoadTexture("textures/powerups/powerup_sticky.png", true, "powerup_sticky");
        // set render-specific controls
        Shader renderershader = ResourceManager::GetShader("sprite");
        Renderer = new SpriteRenderer(renderershader);
        Shader particleshader = ResourceManager::GetShader("particle");
        Particles = new ParticleGenerator(particleshader, ResourceManager::GetTexture("particle"), 500);
        Shader effectshader = ResourceManager::GetShader("postprocessing");
        Effects = new PostProcessor(effectshader, this->Width, this->Height);
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
        // disable all active powerups
        Effects->Chaos = Effects->Confuse = false;
        Ball->PassThrough = Ball->Sticky = false;
        Player->Color = glm::vec3(1.0f);
        Ball->Color = glm::vec3(1.0f);
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
                    {
                        box.Destroyed = true;
                        this->SpawnPowerUps(box);
                    }
                    else
                    {   // if block is solid, enable shake effect
                        ShakeTime = 0.05f;
                        Effects->Shake = true;
                    }
                    // collision resolution
                    Direction dir = std::get<1>(collision);
                    glm::vec2 diff_vector = std::get<2>(collision);
                    if (!(Ball->PassThrough && !box.IsSolid)) // don't do collision resolution on non-solid bricks if pass-through is activated
                    {
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
                                Ball->Position.y -= penetration; // move ball bback up
                            else
                                Ball->Position.y += penetration; // move ball back down
                        }
                    }
                }
            }    
        }
        
        // also check collisions on PowerUps and if so, activate them
        for (PowerUp &powerUp : this->PowerUps)
        {
            if (!powerUp.Destroyed)
            {
                // first check if powerup passed bottom edge, if so: keep as inactive and destroy
                if (powerUp.Position.y >= this->Height)
                    powerUp.Destroyed = true;

                if (CheckCollision(*Player, powerUp))
                {	// collided with player, now activate powerup
                    ActivatePowerUp(powerUp);
                    powerUp.Destroyed = true;
                    powerUp.Activated = true;
                }
            }
        }
        
        // and finally check collisions for player pad (unless stuck)
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
            //Ball->Velocity.y = -Ball->Velocity.y;
            Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
            // fix sticky paddle
            Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
            
            // if Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
            Ball->Stuck = Ball->Sticky;
        }
    }  
    
    // 파워업 생성
    void SpawnPowerUps(GameObject &block)
    {
        if (ShouldSpawn(75)) // 1 in 75 chance
            this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
        if (ShouldSpawn(75))
            this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
        if (ShouldSpawn(75))
            this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
        if (ShouldSpawn(75))
            this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
        if (ShouldSpawn(15)) // Negative powerups should spawn more often
            this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
        if (ShouldSpawn(15))
            this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
    } 
    // 파워업 업데이트
    void UpdatePowerUps(float dt)
    {
        for (PowerUp &powerUp : this->PowerUps)
        {
            powerUp.Position += powerUp.Velocity * dt;
            if (powerUp.Activated)
            {
                powerUp.Duration -= dt;

                if (powerUp.Duration <= 0.0f)
                {
                    // remove powerup from list (will later be removed)
                    powerUp.Activated = false;
                    // deactivate effects
                    if (powerUp.Type == "sticky")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
                        {	// only reset if no other PowerUp of type sticky is active
                            Ball->Sticky = false;
                            Player->Color = glm::vec3(1.0f);
                        }
                    }
                    else if (powerUp.Type == "pass-through")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
                        {	// only reset if no other PowerUp of type pass-through is active
                            Ball->PassThrough = false;
                            Ball->Color = glm::vec3(1.0f);
                        }
                    }
                    else if (powerUp.Type == "confuse")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                        {	// only reset if no other PowerUp of type confuse is active
                            Effects->Confuse = false;
                        }
                    }
                    else if (powerUp.Type == "chaos")
                    {
                        if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                        {	// only reset if no other PowerUp of type chaos is active
                            Effects->Chaos = false;
                        }
                    }
                }
            }
        }
        // Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
        // Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
        this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
            [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
        ), this->PowerUps.end());
    }
    // 파워업 활성화
    void ActivatePowerUp(PowerUp &powerUp)
    {
        if (powerUp.Type == "speed")
        {
            Ball->Velocity *= 1.2;
        }
        else if (powerUp.Type == "sticky")
        {
            Ball->Sticky = true;
            Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
        }
        else if (powerUp.Type == "pass-through")
        {
            Ball->PassThrough = true;
            Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
        }
        else if (powerUp.Type == "pad-size-increase")
        {
            Player->Size.x += 50;
        }
        else if (powerUp.Type == "confuse")
        {
            if (!Effects->Chaos)
                Effects->Confuse = true; // only activate if chaos wasn't already active
        }
        else if (powerUp.Type == "chaos")
        {
            if (!Effects->Confuse)
                Effects->Chaos = true;
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
        // 파워업 업데이트
        this->UpdatePowerUps(dt);
        // 후처리 효과 흔들림효과 숫자가 0이 될 때 까지 흔들림을 생성함
        if(ShakeTime > 0.0f)
        {
            ShakeTime -= dt;
            if(ShakeTime <= 0.0f)
                Effects->Shake = false;
        }
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
            // begin rendering to postprocessing framebuffer
            Effects->BeginRender();
                // draw background
                Texture2D background = ResourceManager::GetTexture("background");
                Renderer->DrawSprite(background, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
                // draw level
                this->Levels[this->Level].Draw(*Renderer);
                // draw player
                Player->Draw(*Renderer);
                // draw PowerUps
                for (PowerUp &powerUp : this->PowerUps)
                    if (!powerUp.Destroyed)
                        powerUp.Draw(*Renderer);   
                // draw particlse
                Particles->Draw();
                // draw ball
                Ball->Draw(*Renderer);
            // end rendering to postprocessing framebuffer
            Effects->EndRender();
            // render postprocessing quad
            Effects->Render(glfwGetTime());
        }
    }
};

#endif
