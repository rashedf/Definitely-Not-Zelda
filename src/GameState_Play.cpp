#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <math.h>

GameState_Play::GameState_Play(GameEngine & game, const std::string & levelPath)
    : GameState(game)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void GameState_Play::init(const std::string & levelPath)
{
    loadLevel(levelPath);
}

void GameState_Play::loadLevel(const std::string & filename)
{
	// Input file
	std::ifstream levelFile(filename);

	std::string animationName, token, aibehav;
	Vec2 patrolPos;
	int tileposX, tileposY, roomposX, roomposY, blockM, blockV, patrolPosNumber;
	float followSpeed, patrolSpeed;
	
	while (levelFile.good()) {
		levelFile >> token;

		// Store player config values 
		if (token == "Player") {
			levelFile >> m_playerConfig.X >> m_playerConfig.Y
				>> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED;
		}
		// Create a tile entity using the config values
		if (token == "Tile") {
			auto tile = m_entityManager.addEntity("tile");
			levelFile >> animationName >> roomposX >> roomposY >> tileposX >> tileposY >> blockM >> blockV;
			auto roomOrigin		= Vec2(m_game.window().getSize().x * (float)roomposX, m_game.window().getSize().y * (float)roomposY);
			auto tilePos		= Vec2(tileposX, tileposY);
			auto animation		= m_game.getAssets().getAnimation(animationName);
			auto tileRoomPos	= roomOrigin + (tilePos * animation.getSize().x);

			tile->addComponent<CAnimation>	(animation, true);
			tile->addComponent<CBoundingBox>(animation.getSize(), blockM, blockV);
			tile->addComponent<CTransform>	(tileRoomPos + tile->getComponent<CBoundingBox>()->halfSize);
		}
		// Create an NPC entity using the config values
		if (token == "NPC") {
			auto npc = m_entityManager.addEntity("npc");
			levelFile >> animationName >> roomposX >> roomposY >> tileposX >> tileposY >> blockM >> blockV >> aibehav;
			auto roomOrigin		= Vec2(m_game.window().getSize().x * (float)roomposX, m_game.window().getSize().y * (float)roomposY);
			auto tilePos		= Vec2(tileposX, tileposY);
			auto animation		= m_game.getAssets().getAnimation(animationName);
			auto NPCRoomPos		= roomOrigin + (tilePos * animation.getSize().x);

			npc->addComponent<CBoundingBox>	(m_game.getAssets().getAnimation(animationName).getSize(), blockM, blockV);
			npc->addComponent<CTransform>	(NPCRoomPos + npc->getComponent<CBoundingBox>()->halfSize);
			npc->addComponent<CAnimation>	(animation, true);
			
			// If the NPC is patrol-type
			if (aibehav == "Patrol") {
				levelFile >> patrolSpeed >> patrolPosNumber;
				std::vector<Vec2> positions;

				npc->addComponent<CPatrol>(positions, patrolSpeed);
				
				// Store the patrol positions in the vector included in the CPatrol component
				for (auto i = 0; i < patrolPosNumber; i++) {
					levelFile >> patrolPos.x >> patrolPos.y;
					auto patrolRoomPos = roomOrigin + (patrolPos * animation.getSize().x);
					npc->getComponent<CPatrol>()->positions.push_back(patrolRoomPos + npc->getComponent<CBoundingBox>()->halfSize);
				}	
			}
			// If the NPC is a follow-type
			if (aibehav == "Follow") {
				levelFile >> followSpeed;
				npc->addComponent<CFollowPlayer>(npc->getComponent<CTransform>()->pos, followSpeed);
				npc->getComponent<CFollowPlayer>()->home = npc->getComponent<CTransform>()->pos;
			}
		}

	}

    // spawn the player at the start of the game
    spawnPlayer();
}

void GameState_Play::spawnPlayer()
{
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>	(Vec2(m_playerConfig.X, m_playerConfig.Y));
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), 0, 0);
    m_player->addComponent<CAnimation>	(m_game.getAssets().getAnimation("StandDown"), true);
    m_player->addComponent<CInput>		();
    
    // New element to CTransform: 'facing', to keep track of where the player is facing
    m_player->getComponent<CTransform>()->facing = Vec2(0, 1);
}

void GameState_Play::spawnSword(std::shared_ptr<Entity> entity)
{
	if (m_entityManager.getEntities("sword").size() == 0) {
		auto eTransform					= entity->getComponent<CTransform>();
		auto sword						= m_entityManager.addEntity("sword");
		std::string sword_animations[]	= {"SwordRight", "SwordUp"};

		sword->addComponent<CAnimation>		(m_game.getAssets().getAnimation(sword_animations[eTransform->facing.y != 0]), true);
		sword->addComponent<CBoundingBox>	(sword->getComponent<CAnimation>()->animation.getSize(), 0, 0);
		sword->addComponent<CTransform>		(eTransform->pos + (eTransform->facing * (entity->getComponent<CBoundingBox>()->halfSize.x + sword->getComponent<CBoundingBox>()->halfSize.x)));
		sword->addComponent<CLifeSpan>		(150);
		if (eTransform->facing.x != 0) {
			sword->getComponent<CTransform>()->scale.x = eTransform->facing.x;
		}
		if (eTransform->facing.y != 0) {
			sword->getComponent<CTransform>()->scale.y = -eTransform->facing.y;
		}
	}
	
}

void GameState_Play::update()
{
    m_entityManager.update();

	// Pause/resume functionality
    if (!m_paused)
    {
        sAI();
        sMovement();
        sLifespan();
        sCollision();
        sAnimation();
    }

    sUserInput();
    sRender();
}

void GameState_Play::sMovement()
{
	auto player_movement	= m_player->getComponent<CInput>();
	auto player_transform	= m_player->getComponent<CTransform>();
	auto player_facing		= m_player->getComponent<CTransform>()->facing;

	// Stop moving in the y direction when there is no up/down input or the current input is left/right
	if (!player_movement->up && !player_movement->down || player_movement->left || player_movement->right) {
		player_transform->speed.y = 0;
	}
	// Stop moving in the x direction when there is no left/right input or the current input is up/down
	if (!player_movement->left && !player_movement->right || player_movement->up || player_movement->down) {
		player_transform->speed.x = 0;
	}

	// LEFT input
	if (player_movement->left) {
		player_transform->speed.x = -m_playerConfig.SPEED;
		player_transform->scale.x = -1;
		player_facing = Vec2(-1, 0);
	}
	// RIGHT input
	else if (player_movement->right) {
		player_transform->speed.x = m_playerConfig.SPEED;
		player_transform->scale.x = 1;
		player_facing = Vec2(1, 0);
	}
	// UP input
	else if (player_movement->up) {
		player_transform->speed.y = -m_playerConfig.SPEED;
		player_facing = Vec2(0, -1);
	}
	// DOWN input
	else if (player_movement->down) {
		player_transform->speed.y = m_playerConfig.SPEED;
		player_facing = Vec2(0, 1);
	}

	player_transform->prevPos = player_transform->pos;
	player_transform->pos += player_transform->speed;
	m_player->getComponent<CTransform>()->facing = player_facing;

	// update sword's position so that the sword moves with the player
	for (auto sword : m_entityManager.getEntities("sword")) {
		sword->getComponent<CTransform>()->pos = (player_transform->pos + (player_transform->facing * (m_player->getComponent<CBoundingBox>()->halfSize.x + sword->getComponent<CBoundingBox>()->halfSize.x)));
	}
}

void GameState_Play::sAI()
{
	auto npcs = m_entityManager.getEntities("npc");

	for (auto npc : npcs) {
		// Patrol NPC :
		// Move the NPC from current position to the next position using the positions vector in the CPatrol component
		// When the last patrol position has been reached, go to the first position and repeat
		if (npc->hasComponent<CPatrol>()) {
			auto patrol			= npc->getComponent<CPatrol>();
			auto transform		= npc->getComponent<CTransform>();
			auto nextPosition	= (patrol->currentPosition + 1) % int(patrol->positions.size());
			auto direction		= patrol->positions[nextPosition] - patrol->positions[patrol->currentPosition];

			transform->pos	+= Vec2(patrol->speed * ((direction.x > 0) - (direction.x < 0)), patrol->speed * ((direction.y > 0) - (direction.y < 0)));
			if (transform->pos.dist(patrol->positions[nextPosition]) <= 5) {
				patrol->currentPosition = nextPosition;
			}
		}
		// Follow NPC
		// If there are no vision-blocking entities in the way, set goal of NPC to player, otherwise set goal to home using the Vec2 in CFollowPlayer component
		if (npc->hasComponent<CFollowPlayer>()) {
			auto followPlayer		= npc->getComponent<CFollowPlayer>();
			auto transform			= npc->getComponent<CTransform>();
			auto player_transform	= m_player->getComponent<CTransform>();
			bool follow				= true;
			
			// Check for vision-blocking entities
			for (auto entity : m_entityManager.getEntities()) {
				if (entity->hasComponent<CBoundingBox>() && entity->getComponent<CBoundingBox>()->blockVision) {
					if (Physics::EntityIntersect(transform->pos, player_transform->pos, entity)) {
						follow = false;
						break;
					}
				}
			}

			// set goal to player (default behavior)
			auto direction	= player_transform->pos - transform->pos;
			// set goal to home if vision is blocked
			if (!follow) {
				if (transform->pos.dist(followPlayer->home) > 5.0f) {		// stop heading to home if npc is within 5 pixels of home. This prevents the NPC from oscilating around or overshooting the target
					direction = followPlayer->home - transform->pos;
				}
				else {
					direction *= 0;
				}
			}
			
			// move towards goal with speed equal to the ratio of the vector to goal
			float speedx = followPlayer->speed;
			float speedy = followPlayer->speed;
			// if x distance is larger, change y speed to the fraction of actual speed according to the ratio
			if (abs(direction.x) > abs(direction.y)) {
				speedy = abs(speedy * ((float)direction.y / (float)direction.x));
			}
			// if y distance is larger, change x speed to the fraction of actual speed according to the ratio
			else if (abs(direction.x) < abs(direction.y)) {
				speedx = abs(speedx * ((float)direction.x / (float)direction.y));
			}
			
			transform->prevPos	 = transform->pos;
			transform->pos		+= Vec2(speedx * ((direction.x > 0) - (direction.x < 0)), speedy * ((direction.y > 0) - (direction.y < 0)));
		}
	}
}

void GameState_Play::sLifespan()
{
	// check for entities with a lifespan and destroy them if they have exceeded their lifespan
	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CLifeSpan>()) {
			auto lifespan = e->getComponent<CLifeSpan>();
			if (lifespan->clock.getElapsedTime().asMilliseconds() >= lifespan->lifespan) {
				e->destroy();
			}
		}
	}
}

void GameState_Play::sCollision()
{
	auto tiles				= m_entityManager.getEntities("tile");
	auto player_transform	= m_player->getComponent<CTransform>();

	// Check tile collisions
	for (auto tile : tiles) {

		// Tile with player
		auto current_overlap	= Physics::GetOverlap(tile, m_player);
		auto previous_overlap	= Physics::GetPreviousOverlap(tile, m_player);

		if (current_overlap.x > 0 && current_overlap.y > 0 && tile->getComponent<CBoundingBox>()->blockMove) {
			float delta_y = player_transform->prevPos.y - player_transform->pos.y;
			float delta_x = player_transform->prevPos.x - player_transform->pos.x;

			// If player came from above/below the tile
			if (previous_overlap.x > 0) {
				player_transform->pos.y += current_overlap.y * ((delta_y > 0) - (delta_y < 0));
			}
			// If player came from left/right of the tile
			else if (previous_overlap.y > 0) {
				player_transform->pos.x += current_overlap.x * ((delta_x > 0) - (delta_x < 0));
			}
		}

		// Check NPC collisions
		for (auto npc : m_entityManager.getEntities("npc")) {

			auto npc_transform		= npc->getComponent<CTransform>();
			current_overlap			= Physics::GetOverlap(tile, npc);
			previous_overlap		= Physics::GetPreviousOverlap(tile, npc);
			auto player_npc_overlap = Physics::GetOverlap(npc, m_player);

			// Player with NPC
			if (player_npc_overlap.x > 0 && player_npc_overlap.y > 0) {
				m_player->destroy();
				spawnPlayer();
			}

			// Tile with NPC
			if (current_overlap.x > 0 && current_overlap.y > 0 && tile->getComponent<CBoundingBox>()->blockMove) {
				float delta_y = npc_transform->prevPos.y - npc_transform->pos.y;
				float delta_x = npc_transform->prevPos.x - npc_transform->pos.x;

				if (previous_overlap.x > 0) {
					npc_transform->pos.y += current_overlap.y * ((delta_y > 0) - (delta_y < 0));
				}
				else if (previous_overlap.y > 0) {
					npc_transform->pos.x += current_overlap.x * ((delta_x > 0) - (delta_x < 0));
				}
			}

			// Sword with NPC
			for (auto sword : m_entityManager.getEntities("sword")) {
				auto sword_npc_overlap = Physics::GetOverlap(npc, sword);

				// destroy the NPC and play the explosion animation
				if (sword_npc_overlap.x > 0 && sword_npc_overlap.y > 0) {
					auto explosion = m_entityManager.addEntity("explosions");
					explosion->addComponent<CAnimation>(m_game.getAssets().getAnimation("Explosion"), false);
					explosion->addComponent<CTransform>(npc_transform->pos);
					explosion->getComponent<CTransform>()->scale *= 0.8;
					npc->destroy();
				}
			}
		}
	}
}

void GameState_Play::sAnimation()
{
	auto player_transform	= m_player->getComponent<CTransform>();
	auto player_animation	= m_player->getComponent<CAnimation>();
	bool hasSword			= m_entityManager.getEntities("sword").size() > 0;
	auto animation			= "StandDown";

	// If player is stationary
	if (player_transform->pos == player_transform->prevPos) {
		if (player_transform->facing.x == 0) {
			if (player_transform->facing.y > 0) {
				animation = "StandDown";
			}
			else {
				animation = "StandUp";
			}
		}
		else {
			animation = "StandRight";
		}
		player_animation->animation = m_game.getAssets().getAnimation(animation);
	}
	// If player is moving
	else {
		if (player_transform->facing.x == 0) {
			if (player_transform->facing.y > 0 && player_animation->animation.getName() != "RunDown") {
				player_animation->animation = m_game.getAssets().getAnimation("RunDown");
			}
			else if (player_transform->facing.y < 0 && player_animation->animation.getName() != "RunUp"){
				player_animation->animation = m_game.getAssets().getAnimation("RunUp");
			}
		}
		else if (player_transform->facing.y == 0 && player_animation->animation.getName() != "RunRight") {
			player_animation->animation = m_game.getAssets().getAnimation("RunRight");
		}
	}
	// If player is attacking
	if (hasSword) {
		if (player_transform->facing.x == 0) {
			if (player_transform->facing.y > 0) {
				animation = "AtkDown";
			}
			else {
				animation = "AtkUp";
			}
		}
		else {
			animation = "AtkRight";
		}
		player_animation->animation = m_game.getAssets().getAnimation(animation);
	}

	// Update all animations and destroy entities with a non-repeating animation that has ended
	for (auto entity : m_entityManager.getEntities()) {
		if (entity->hasComponent<CAnimation>()) {
			entity->getComponent<CAnimation>()->animation.update();
			if (!entity->getComponent<CAnimation>()->repeat && entity->getComponent<CAnimation>()->animation.hasEnded()) {
				entity->destroy();
			}
		}
	}
	
}

void GameState_Play::sUserInput()
{
    auto pInput = m_player->getComponent<CInput>();

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
                case sf::Keyboard::Escape:  { m_game.popState(); break; }
                case sf::Keyboard::W:       { pInput->up = true; break; }
                case sf::Keyboard::A:       { pInput->left = true; break; }
                case sf::Keyboard::S:       { pInput->down = true; break; }
                case sf::Keyboard::D:       { pInput->right = true; break; }
                case sf::Keyboard::Z:       { init(m_levelPath); break; }
                case sf::Keyboard::R:       { m_drawTextures = !m_drawTextures; break; }
                case sf::Keyboard::F:       { m_drawCollision = !m_drawCollision; break; }
                case sf::Keyboard::Y:       { m_follow = !m_follow; break; }
                case sf::Keyboard::P:       { setPaused(!m_paused); break; }
                case sf::Keyboard::Space:   { spawnSword(m_player); break; }
            }
        }

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:       { pInput->up = false; break; }
                case sf::Keyboard::A:       { pInput->left = false; break; }
                case sf::Keyboard::S:       { pInput->down = false; break; }
                case sf::Keyboard::D:       { pInput->right = false; break; }
                case sf::Keyboard::Space:   { pInput->shoot = false; pInput->canShoot = true; break; }
            }
        }
    }
}

void GameState_Play::sRender()
{
    m_game.window().clear(sf::Color(255, 192, 122));

    // set the window view 
	auto playerPosition = m_player->getComponent<CTransform>()->pos;
	auto windowSize		= m_game.window().getSize();
	unsigned int size	= 100;
	sf::View view		= m_game.window().getView();
	sf::View minimap	(sf::FloatRect(view.getCenter().x, view.getCenter().y, size, m_game.window().getSize().y*size / m_game.window().getSize().x));
	minimap.setViewport (sf::FloatRect(1.f - (1.f*minimap.getSize().x) / m_game.window().getSize().x - 0.02f, 1.f - (1.f*minimap.getSize().y) / m_game.window().getSize().y - 0.02f, (1.f*minimap.getSize().x) / m_game.window().getSize().x, (1.f*minimap.getSize().y) / m_game.window().getSize().y));
	minimap.zoom(4.f);

	if (m_follow) {
		view.setCenter(playerPosition.x, playerPosition.y);
	}
	else {
		view.setCenter(floor(playerPosition.x / windowSize.x) * windowSize.x + windowSize.x / 2.0f, floor(playerPosition.y / windowSize.y) * windowSize.y + windowSize.y / 2.0f);
	}
	
	m_game.window().setView(view);
	drawMap();

	// Attempt at creating a minimap
	/*
	m_game.window().setView(minimap);
	drawMap();
    */

    m_game.window().display();
}

void GameState_Play::drawMap() {
	// draw all Entity textures / animations
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto animation = e->getComponent<CAnimation>()->animation;
				animation.getSprite().setRotation(transform->angle);
				animation.getSprite().setPosition(transform->pos.x, transform->pos.y);
				animation.getSprite().setScale(transform->scale.x, transform->scale.y);
				m_game.window().draw(animation.getSprite());
			}
		}
	}

	// draw all Entity collision bounding boxes with a rectangleshape
	if (m_drawCollision)
	{
		sf::CircleShape dot(4);
		dot.setFillColor(sf::Color::Black);
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto box = e->getComponent<CBoundingBox>();
				auto transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box->size.x - 1, box->size.y - 1));
				rect.setOrigin(sf::Vector2f(box->halfSize.x, box->halfSize.y));
				rect.setPosition(transform->pos.x, transform->pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));

				if (box->blockMove && box->blockVision) { rect.setOutlineColor(sf::Color::Black); }
				if (box->blockMove && !box->blockVision) { rect.setOutlineColor(sf::Color::Blue); }
				if (!box->blockMove && box->blockVision) { rect.setOutlineColor(sf::Color::Red); }
				if (!box->blockMove && !box->blockVision) { rect.setOutlineColor(sf::Color::White); }
				rect.setOutlineThickness(1);
				m_game.window().draw(rect);
			}

			if (e->hasComponent<CPatrol>())
			{
				auto & patrol = e->getComponent<CPatrol>()->positions;
				for (size_t p = 0; p < patrol.size(); p++)
				{
					dot.setPosition(patrol[p].x, patrol[p].y);
					m_game.window().draw(dot);
				}
			}

			if (e->hasComponent<CFollowPlayer>())
			{
				sf::VertexArray lines(sf::LinesStrip, 2);
				lines[0].position.x = e->getComponent<CTransform>()->pos.x;
				lines[0].position.y = e->getComponent<CTransform>()->pos.y;
				lines[0].color = sf::Color::Black;
				lines[1].position.x = m_player->getComponent<CTransform>()->pos.x;
				lines[1].position.y = m_player->getComponent<CTransform>()->pos.y;
				lines[1].color = sf::Color::Black;
				m_game.window().draw(lines);
				dot.setPosition(e->getComponent<CFollowPlayer>()->home.x, e->getComponent<CFollowPlayer>()->home.y);
				m_game.window().draw(dot);
			}
		}
	}
}
