#include "GameState_Menu.h"
#include "GameState_Play.h"
#include "Common.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"

GameState_Menu::GameState_Menu(GameEngine & game)
    : GameState(game)
{
    init("");
}

void GameState_Menu::init(const std::string & menuConfig)
{
    m_title = "Definitely Not Zelda";
    m_menuStrings.push_back("Level  1");
    m_menuStrings.push_back("Level  2");
    m_menuStrings.push_back("Level  3");

    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("level2.txt");
    m_levelPaths.push_back("level3.txt");

    m_menuText.setFont(m_game.getAssets().getFont("Megaman"));
    m_menuText.setCharacterSize(64);
}

void GameState_Menu::update()
{
    m_entityManager.update();

    sUserInput();
    sRender();
}

void GameState_Menu::sUserInput()
{
    sf::Event event;
    while (m_game.window().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_game.quit();
        }
        // this event is triggered when a key is pressed
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape: 
                { 
                    m_game.quit(); 
                    break; 
                }
                case sf::Keyboard::W: 
                {
                    if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
                    else { m_selectedMenuIndex = m_menuStrings.size() - 1; }
                    break;
                }
                case sf::Keyboard::S: 
                { 
                    m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size(); 
                    break; 
                }
                case sf::Keyboard::D: 
                { 
                    m_game.pushState(std::make_shared<GameState_Play>(m_game, m_levelPaths[m_selectedMenuIndex]));
                    break; 
                }
            }
        }
    }

}

void GameState_Menu::sRender()
{
    // clear the window to a blue
    m_game.window().setView(m_game.window().getDefaultView());
    m_game.window().clear(sf::Color(0, 0, 0));

    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(48);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setPosition(sf::Vector2f(10, 10));
    m_game.window().draw(m_menuText);
    
    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color(100, 100, 100));
        m_menuText.setPosition(sf::Vector2f(10, 110 + i * 72));
        m_game.window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(20);
    m_menuText.setFillColor(sf::Color(100, 100, 100));
    m_menuText.setString("up: w     down: s    play: d      back: esc");
    m_menuText.setPosition(sf::Vector2f(10, 730));
    m_game.window().draw(m_menuText);

    m_game.window().display();
}