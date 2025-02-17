#include "CCL.h"
#include "GAME/GameComponents.h"
#include "DRAW/DrawComponents.h"
#include "UTIL/Utilities.h"

namespace GAME 
{
	void UpdateTransforms(entt::registry& registry)
	{
		// Update all transforms
		auto view = registry.view<Transform, DRAW::MeshCollection>();
		
		for (auto entity : view)
		{
			auto transform = view.get<Transform>(entity);
			auto meshes = view.get<DRAW::MeshCollection>(entity);

			for (auto mesh : meshes.meshes)
			{
				if (registry.valid(mesh) && registry.all_of<DRAW::GPUInstance>(mesh))
				{
					auto& instance = registry.get<DRAW::GPUInstance>(mesh);
					instance.transform = transform.transform;
				}
			}
		}
	}

	void UpdateVelocity(entt::registry& registry)
	{
		auto& deltaTime = registry.ctx().get<UTIL::DeltaTime>();

		auto view = registry.view<Transform, Velocity>();
		for each (auto entity in view)
		{
			auto& transform = view.get<Transform>(entity);
			auto velocity = view.get<Velocity>(entity);
			GW::MATH::GVector::ScaleF(velocity.velocity, deltaTime.dtSec, velocity.velocity);
			GW::MATH::GMatrix::TranslateGlobalF(transform.transform, velocity.velocity, transform.transform);
		}
	}
	void HandleCollision(entt::registry& registry, entt::entity& entity, entt::entity& otherEntity, GW::MATH::GOBBF boundingBox, GW::MATH::GOBBF otherBoundingBox)
	{
		bool isProjectile = registry.all_of<Collidable, Projectile>(entity);
		bool isWall = registry.all_of<Collidable, Obstacle>(otherEntity);

		if (isProjectile && isWall)
		{
			registry.emplace<ToDestroy>(entity);
			return;
		}

		bool isEnemy = registry.all_of<Collidable, Enemy>(entity);

		if (isEnemy && isWall)
		{
			//TODO: Handle enemy collision with wall
			auto& velocity = registry.get<Velocity>(entity);
			GW::MATH::GVECTORF normal = { 0.0f, 0.0f, 0.0f , 0.0f};
			GW::MATH::GCollision::ClosestPointToOBBF(otherBoundingBox, boundingBox.center, normal);

			GW::MATH::GVector::SubtractVectorF(boundingBox.center, normal,  normal);
			normal = GW::MATH::GVECTORF{ normal.x, 0.0f, normal.z, 0.0f };
			GW::MATH::GVector::NormalizeF(normal, normal);

			float calculation = 0.0f;
			GW::MATH::GVector::DotF(velocity.velocity, normal , calculation);
			GW::MATH::GVector::ScaleF(normal, calculation * 2.0f , normal);
			GW::MATH::GVector::SubtractVectorF(velocity.velocity, normal, velocity.velocity);
			return;
		}

		bool isPlayer = registry.all_of<Collidable, Player>(otherEntity);
		if (isPlayer && isWall)
		{
			std::cout << "Player has collided with a wall!" << std::endl;
		}
		if (isEnemy && isPlayer)
		{
			auto& health = registry.get<Health>(otherEntity);
			if (registry.all_of<Invulnerable>(otherEntity) == false) 
			{
				std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
				health.health -= 1;
				UTIL::UpdateUILives(registry, health.health);
				std::cout << "Player Hit! Current HP: " << health.health << std::endl;
				registry.emplace<Invulnerable>(otherEntity, Invulnerable{ config.get()->at("Player").at("invulnPeriod").as<float>() });
			}
			return;
		}

		isProjectile = registry.all_of<Collidable, Projectile>(otherEntity);

		if (isEnemy && isProjectile)
		{
			auto& health = registry.get<Health>(entity);
			health.health -= 1;
			registry.emplace_or_replace<ToDestroy>(otherEntity);
			return;
		}		
	}

	void CheckForCollistions(entt::registry& registry)
	{
		auto view = registry.view<Collidable, Transform, DRAW::MeshCollection>();
		for each (auto entity in view)
		{
			if (!registry.valid(entity)) {
				continue;
			}
			auto& transform = view.get<Transform>(entity);
			auto& meshes = view.get<DRAW::MeshCollection>(entity);
			
			for each (auto otherEntity in view)
			{
				if (entity != otherEntity)
				{
					if (!registry.valid(entity)) {
						continue;
					}
					GW::MATH::GOBBF boundingBox = meshes.boundingBox;
					auto otherTransform = view.get<Transform>(otherEntity);
					auto otherMeshes = view.get<DRAW::MeshCollection>(otherEntity);
					GW::MATH::GOBBF otherBoundingBox = otherMeshes.boundingBox;
					GW::MATH::GVECTORF scale = { 1.0f, 1.0f, 1.0f };
					GW::MATH::GVECTORF otherScale = { 1.0f, 1.0f, 1.0f };

					GW::MATH::GMatrix::GetScaleF(transform.transform, scale);
					boundingBox.extent.x *= scale.x;
					boundingBox.extent.y *= scale.y;
					boundingBox.extent.z *= scale.z;
					GW::MATH::GMatrix::GetScaleF(otherTransform.transform, otherScale);
					otherBoundingBox.extent.x *= otherScale.x;
					otherBoundingBox.extent.y *= otherScale.y;
					otherBoundingBox.extent.z *= otherScale.z;

					GW::MATH::GMatrix::VectorXMatrixF(transform.transform, boundingBox.center, boundingBox.center);
					GW::MATH::GMatrix::VectorXMatrixF(otherTransform.transform, otherBoundingBox.center, otherBoundingBox.center);


					GW::MATH::GQUATERNIONF rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
					GW::MATH::GQUATERNIONF otherRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
					GW::MATH::GQuaternion::SetByMatrixF(transform.transform, rotation);
					GW::MATH::GQuaternion::SetByMatrixF(otherTransform.transform, otherRotation);

					GW::MATH::GQuaternion::MultiplyQuaternionF(boundingBox.rotation, rotation, boundingBox.rotation);
					GW::MATH::GQuaternion::MultiplyQuaternionF(otherBoundingBox.rotation, otherRotation, otherBoundingBox.rotation);

					GW::MATH::GCollision::GCollisionCheck collisionCheck = GW::MATH::GCollision::GCollisionCheck::NO_COLLISION;
					GW::MATH::GCollision::TestOBBToOBBF(boundingBox, otherBoundingBox, collisionCheck);

					if (collisionCheck == GW::MATH::GCollision::GCollisionCheck::COLLISION)
					{
						HandleCollision(registry, entity, otherEntity, boundingBox, otherBoundingBox);
					}
				}
			}
		}
	}

	void UpdateEnemyState(entt::registry& registry)
	{
		std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
		auto& view = registry.view<Enemy, Health>();

		for (auto entity : view)
		{
			auto& health = view.get<Health>(entity);
			if (health.health <= 0)
			{
				if (registry.ctx().contains<GW::AUDIO::GAudio>())
				{
					auto& gAudio = registry.ctx().get<GW::AUDIO::GAudio>();

					std::vector<std::string> explosionSounds = {
						(*config).at("SFX").at("asExplosion1").as<std::string>(),
						(*config).at("SFX").at("asExplosion2").as<std::string>(),
						(*config).at("SFX").at("asExplosion3").as<std::string>(),
						(*config).at("SFX").at("asExplosion4").as<std::string>()
					};

					int randIndex = rand() % explosionSounds.size();
					std::string selectedSound = explosionSounds[randIndex];

					auto explosionEntity = registry.create();
					auto& explosionSFX = registry.emplace<GAME::Explosion>(explosionEntity).explosion;
					explosionSFX.Create(selectedSound.c_str(), gAudio);
					explosionSFX.Play();
				}

				UTIL::UpdateUIActiveScore(registry, 100);
				registry.emplace<ToDestroy>(entity);

				if (registry.all_of<Shatters>(entity))
				{
					Shatters shatters = registry.get<Shatters>(entity);
					shatters.shatterCount -= 1;
					Transform transform = registry.get<Transform>(entity);
					int shatterAmount = config.get()->at("Enemy1").at("shatterAmount").as<int>();

					for (int i = 0; i < shatterAmount; i++)
					{
						auto newEnemy = registry.create();
						std::vector<std::string> enemyKeys = { "Enemy1", "Enemy2", "Enemy3", "Enemy4", "Enemy5" };
						int randIndex = rand() % enemyKeys.size();
						std::string selectedEnemy = enemyKeys[randIndex];

						registry.emplace<Enemy>(newEnemy);
						registry.emplace<Collidable>(newEnemy);
						auto& enemyMeshes = registry.emplace<DRAW::MeshCollection>(newEnemy);
						auto& enemyTransform = registry.emplace<Transform>(newEnemy);
						auto& enemyVelocity = registry.emplace<Velocity>(newEnemy, Velocity{ UTIL::GetRandomVelocityVector() });

						registry.emplace<Health>(newEnemy, Health{ config.get()->at(selectedEnemy).at("hitpoints").as<int>() });

						if (shatters.shatterCount > 0)
							registry.emplace<Shatters>(newEnemy, Shatters{ shatters.shatterCount });

						GW::MATH::GVector::ScaleF(enemyVelocity.velocity, config.get()->at(selectedEnemy).at("speed").as<float>(), enemyVelocity.velocity);

						auto enemyModel = config.get()->at(selectedEnemy).at("model").as<std::string>();
						UTIL::CreateDynamicObjects(registry, enemyModel, enemyMeshes, enemyTransform);

						GW::MATH::GVECTORF scaleVector = { 1.0f, 1.0f, 1.0f, 0.0f };
						GW::MATH::GVector::ScaleF(scaleVector, config.get()->at(selectedEnemy).at("shatterScale").as<float>(), scaleVector);
						enemyTransform.transform = transform.transform;
						GW::MATH::GMatrix::ScaleGlobalF(enemyTransform.transform, scaleVector, enemyTransform.transform);

						enemyMeshes.boundingBox = enemyMeshes.boundingBox;
					}
				}
			}
		}
	}


	void DestroyTaggedEntities(entt::registry& registry)
	{
		auto view = registry.view<ToDestroy>();
		for each (auto entity in view)
		{
			registry.destroy(entity);
		}
	}

	void CheckAllPlayersHealth(entt::registry& registry)
	{
		auto view = registry.view<Player, Health>();
		int playersChecked = 0;
		int playersDead = 0;
		for each (auto entity in view)
		{
			auto& health = view.get<Health>(entity);
			if (health.health <= 0)
			{
				playersDead++;
			}
			playersChecked++;
		}
		if (playersChecked == playersDead)
		{
			if (registry.ctx().contains<GW::AUDIO::GMusic>())
			{
				auto& currentMusic = registry.ctx().get<GW::AUDIO::GMusic>();
				currentMusic.Stop();
				registry.ctx().erase<GW::AUDIO::GMusic>();
			}

			if (registry.ctx().contains<GW::AUDIO::GAudio>())
			{
				auto& gAudio = registry.ctx().get<GW::AUDIO::GAudio>();
				std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

				std::string shipExplodeSound = (*config).at("SFX").at("shipExplode").as<std::string>();

				auto gameOverEntity = registry.view<GAME::GameManager>().front();
				auto& explosion = registry.emplace<GAME::ShipExplosion>(gameOverEntity).shipExplosion;
				explosion.Create(shipExplodeSound.c_str(), gAudio);
				explosion.Play();
			}

			if (registry.ctx().contains<GW::AUDIO::GAudio>())
			{
				auto& gAudio = registry.ctx().get<GW::AUDIO::GAudio>();
				std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

				std::string gameOverMusic = (*config).at("Music").at("gameOver").as<std::string>();

				auto& gMusic = registry.ctx().emplace<GW::AUDIO::GMusic>();
				gMusic.Create(gameOverMusic.c_str(), gAudio);
				gMusic.Play(true);
			}

			auto gameOver = registry.view<GAME::GameManager>().front();
			registry.emplace<GameOver>(gameOver);
			auto& gameState = registry.ctx().get<GAME::GameState>();
			gameState = GAME::GameState::GAMEOVER;
			std::cout << "You Lose, Game Over!" << std::endl;
		}
	}

	void CheckEnemiesGameOver(entt::registry& registry)
	{
		auto view = registry.view<Enemy>();
		if (view.size() == 0)
		{
			auto gameOver = registry.view<GAME::GameManager>().front();
			registry.emplace<GameOver>(gameOver);
			auto& gameState = registry.ctx().get<GAME::GameState>();
			gameState = GAME::GameState::GAMEOVER;
			std::cout << "You Win, Good Job!" << std::endl;
		}
	}

	void UpdateGame(entt::registry& registry, entt::entity entity)
	{
		UpdateTransforms(registry);
		UpdateVelocity(registry);
		registry.patch<Player>(registry.view<Player>().front());
		registry.patch<UIComponents>(registry.view<UIComponents>().front());
		CheckForCollistions(registry);
		UpdateEnemyState(registry);
		CheckAllPlayersHealth(registry);
		CheckEnemiesGameOver(registry);
		DestroyTaggedEntities(registry);
	}
	
	CONNECT_COMPONENT_LOGIC() 
	{
		registry.on_update<GameManager>().connect<UpdateGame>();
	}
		
}