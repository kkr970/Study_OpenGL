#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//게임 state
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
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
    
    }

    // initialize game state (load all shaders/textures/levels)
    void Init()
    {

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

    }
};

#endif
