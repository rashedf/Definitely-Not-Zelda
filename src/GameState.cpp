#include "GameState.h"
#include "GameEngine.h"

GameState::GameState(GameEngine & game)
    : m_game(game)
{ 
    
}

void GameState::setPaused(bool paused)
{
    m_paused = paused;
}