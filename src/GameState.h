#pragma once

#include "Common.h"
#include <memory>

class GameEngine;

class GameState
{

protected: 
    
    GameEngine &    m_game;
    bool            m_paused = false;

    GameState(GameEngine & game);

public:

    virtual void update() = 0;

    virtual void setPaused(bool paused);
};