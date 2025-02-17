#ifndef UTILITIES_H_
#define UTILITIES_H_

#include "GameConfig.h"
#include "../GAME/gameComponents.h"
#include "../DRAW/DrawComponents.h"

namespace UTIL
{
	struct Config
	{
		std::shared_ptr<GameConfig> gameConfig = std::make_shared<GameConfig>();
	};

	struct DeltaTime
	{
		double dtSec;
	};

	struct Input
	{
		GW::INPUT::GController gamePads; // controller support
		GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
		GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	};

	/// Method declarations

	/// Creates a normalized vector pointing in a random direction on the X/Z plane
	GW::MATH::GVECTORF GetRandomVelocityVector();
	void CreateDynamicObjects(entt::registry& registry, std::string modelName, DRAW::MeshCollection& MeshCollection, GAME::Transform& Transform);
	std::vector<entt::entity> CopyRenderableEntities(entt::registry& registry, std::vector<entt::entity> entitiesToCopy);
	void UpdateUILevel(entt::registry& registry, int level);
	void UpdateUILives(entt::registry& registry, int newLives);
	void UpdateUIActiveScore(entt::registry& registry, int newScore);
	void ResetUIActiveScore(entt::registry& registry);
	void UpdateUIHighScore(entt::registry& registry, int newScore);
	void CheckPausePressed(entt::registry& registry);
	int GetUIActiveScore(entt::registry& registry);
	int GetUIHighScore(entt::registry& registry);


} // namespace UTIL
#endif // !UTILITIES_H_