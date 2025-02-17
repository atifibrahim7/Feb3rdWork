#include "Utilities.h"
#include "../CCL.h"
namespace UTIL
{
	GW::MATH::GVECTORF GetRandomVelocityVector()
	{
		GW::MATH::GVECTORF vel = {float((rand() % 20) - 10), 0.0f, float((rand() % 20) - 10)};
		if (vel.x <= 0.0f && vel.x > -1.0f)
			vel.x = -1.0f;
		else if (vel.x >= 0.0f && vel.x < 1.0f)
			vel.x = 1.0f;

		if (vel.z <= 0.0f && vel.z > -1.0f)
			vel.z = -1.0f;
		else if (vel.z >= 0.0f && vel.z < 1.0f)
			vel.z = 1.0f;

		GW::MATH::GVector::NormalizeF(vel, vel);

		return vel;
	}

	void CreateDynamicObjects(entt::registry& registry, std::string modelName, DRAW::MeshCollection& MeshCollection, GAME::Transform& Transform) {

		auto modelManager = registry.ctx().get<DRAW::ModelManager>();
		bool setTransform = false;

		std::vector<entt::entity> meshes = CopyRenderableEntities(registry, modelManager.models[modelName].meshes);

		for each (auto var in meshes)
		{
			auto entity = registry.create();
			DRAW::GPUInstance gpuInstance = registry.get<DRAW::GPUInstance>(var);
			DRAW::GeometryData geomotryData = registry.get<DRAW::GeometryData>(var);


			registry.emplace<DRAW::GPUInstance>(entity, gpuInstance);
			registry.emplace<DRAW::GeometryData>(entity, geomotryData);

			if (!setTransform)
			{
				Transform.transform = gpuInstance.transform;
				setTransform = true;
			}
			MeshCollection.meshes.push_back(entity);
			GW::MATH::GOBBF boundingBox = modelManager.models[modelName].boundingBox;
			MeshCollection.boundingBox = boundingBox;

		}
	}


	std::vector<entt::entity> CopyRenderableEntities(entt::registry& registry, std::vector<entt::entity> entitiesToCopy) {
		std::vector<entt::entity> newEntities;
		for (auto entity : entitiesToCopy)
		{
			if (!registry.valid(entity) || !registry.all_of<DRAW::GPUInstance>(entity) || !registry.all_of<DRAW::GeometryData>(entity))
			{
				continue;
			}

			auto newEntity = registry.create();
			registry.emplace<DRAW::DoNotRender>(newEntity);
			auto gpuInstance = registry.get<DRAW::GPUInstance>(entity);
			auto geometryData = registry.get<DRAW::GeometryData>(entity);

			DRAW::GPUInstance gpuInstanceCopy = { gpuInstance.transform, gpuInstance.matData };
			DRAW::GeometryData geometryDataCopy = { geometryData.indexStart, geometryData.indexCount, geometryData.vertexStart };

			registry.emplace<DRAW::GPUInstance>(newEntity, gpuInstanceCopy);
			registry.emplace<DRAW::GeometryData>(newEntity, geometryDataCopy);

			newEntities.push_back(newEntity);
		}

		return newEntities;
	}
	void UpdateUILevel(entt::registry& registry, int level)
	{
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			UIElements.currentLevel = level;
		}
	}
	void UpdateUILives(entt::registry& registry, int newLives)
	{
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			UIElements.lives = newLives;
		}
	}
	void UpdateUIActiveScore(entt::registry& registry, int newScore)
	{
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			UIElements.currScore += newScore;
			std::cout << UIElements.currScore;
		}
	}

	void ResetUIActiveScore(entt::registry& registry)
	{
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			UIElements.currScore = 0;
			std::cout << UIElements.currScore;
		}
	}

	int GetUIActiveScore(entt::registry& registry) {
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			return UIElements.currScore;
		}
	}
	void UpdateUIHighScore(entt::registry& registry, int newScore)
	{
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			UIElements.highScore = newScore;
		}
	}
	int GetUIHighScore(entt::registry& registry) {
		auto UIview = registry.view<GAME::UIComponents>();
		if (UIview.begin() != UIview.end()) {
			auto& front = *UIview.begin();
			auto& UIElements = registry.get<GAME::UIComponents>(front);
			return UIElements.highScore;
		}
	}

	void CheckPausePressed(entt::registry& registry) {
		auto& input = registry.ctx().get<UTIL::Input>();
		auto& events = registry.ctx().get<GW::CORE::GEventCache>();
		GW::GEvent e;
		while (+events.Pop(e)) {
			GW::INPUT::GBufferedInput::Events g;
			GW::INPUT::GBufferedInput::EVENT_DATA d;
			if (+e.Read(g, d)) {
				if (g == GW::INPUT::GBufferedInput::Events::KEYPRESSED && d.data == G_KEY_P) {
					auto gameManager = registry.view<GAME::GameManager>().front();
					if (registry.all_of<GAME::Paused>(gameManager) == false) {
						registry.emplace<GAME::Paused>(gameManager);
						std::cout << "Game Paused" << std::endl;
					}
					else {
						registry.remove<GAME::Paused>(gameManager);
						std::cout << "Game Unpaused" << std::endl;
					}
				}
			}
		}
	}
} // namespace UTIL