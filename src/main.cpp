// Rashed Ferdous, rferdous, 201654613
// MD Zayed Imam, zimam, ID:201628005

// What doesn't work: Bonus feature (minimap). Tried to implement a minimap in GameStatePlay::sRender(). Faulty code has been commented out.
// Added custom level on level 2

#include <SFML/Graphics.hpp>

#include "GameEngine.h"

int main()
{
    GameEngine g("assets.txt");
    g.run();
}