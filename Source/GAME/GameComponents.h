#ifndef GAME_COMPONENTS_H_
#define GAME_COMPONENTS_H_

namespace GAME
{
	///*** Tags ***///
	struct Player {
	};
	struct Enemy {
	};
	struct Projectile {
	};
	struct GameManager {
	};
	struct Collidable {
	};
	struct Obstacle {
	};
	struct ToDestroy {
	};
	struct GameOver {
	};
	struct Paused{
	};
	struct cursor1 {
	};
	struct cursor2 {
	};
	struct blinking {
	};

	///*** Components ***///
	struct Transform{
		GW::MATH::GMATRIXF transform;
	};
	
	struct FiringState {
		float fireCoolDown;
	};
	struct Velocity {
		GW::MATH::GVECTORF velocity;
	};
	struct Health {
		int health;
	};
	struct Shatters {
		int shatterCount;
	};
	struct Invulnerable {
		float invulnerableTime;
	};
	struct Rotation 
	{
		float angle = 0.0f;
		float angularVelocity = 0.0f;
	};
	struct Physics 
	{
		GW::MATH::GVECTORF velocity = { 0.0f, 0.0f, 0.0f };
		float thrust = 3.0f;
		float drag = 0.98f;
		float maxSpeed = 5.0f;
	};
	struct UIComponents {
		int lives;
		int currScore;
		int highScore;
		int currentLevel;
	};
	struct PewPew {
		GW::AUDIO::GSound pewPew;
	};
	struct Explosion {
		GW::AUDIO::GSound explosion;
	};
	struct ShipExplosion {
		GW::AUDIO::GSound shipExplosion;
	};
	//*** Game State ***//
	enum class GameState 
	{
		MAIN_MENU,
		GAMEPLAY,
		GAMEOVER
	};
	struct Bounds {
		float minX;
		float maxX;
		float minZ;
		float maxZ;

		Bounds(float minx = 0, float maxx = 0, float minz = 0, float maxz = 0)
			: minX(minx), maxX(maxx), minZ(minz), maxZ(maxz) {
		}
	};

}// namespace GAME
#endif // !GAME_COMPONENTS_H_