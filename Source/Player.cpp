#include "CCL.h"
#include "UTIL/Utilities.h"
#include "GAME/GameComponents.h"

namespace GAME {
	void UpdatePlayer(entt::registry& registry)
	{
		auto& input = registry.ctx().get<UTIL::Input>();
		auto& deltaTime = registry.ctx().get<UTIL::DeltaTime>();
		std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

		auto view = registry.view<Player, Transform>().front();
		auto& transform = registry.get<Transform>(view).transform;

		if (!registry.all_of<Rotation>(view))
		{
			registry.emplace<Rotation>(view, Rotation{ 0.0f, 0.0f });

		}
		auto& rotation = registry.get<Rotation>(view);

		if (!registry.all_of<Physics>(view))
		{
			registry.emplace<Physics>(view);
		}
		auto& physics = registry.get<Physics>(view);

		//Rotation constants
		const float AccelerationForRotate = 1.5f;
		const float dampRotate = 0.55f;
		const float STRONG_DAMPING = 0.55f;
		const float MAX_ANGULAR_VELOCITY = 3.0f;
		const float STOP_THRESHOLD = 0.4f;

		float a = 0.0f, d = 0.0f;
		input.immediateInput.GetState(G_KEY_A, a);
		input.immediateInput.GetState(G_KEY_D, d);

		if (a > 0.0f)
		{
			rotation.angularVelocity += AccelerationForRotate * deltaTime.dtSec;
		}
		else if (d > 0.0f)
		{
			rotation.angularVelocity -= AccelerationForRotate * deltaTime.dtSec;
		}
		else
		{
			rotation.angularVelocity = 0.0f;
			rotation.angle = 0;
		}
		rotation.angularVelocity = std::clamp(rotation.angularVelocity, -MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);

		if (a == 0.0f && d == 0.0f)
		{
			rotation.angularVelocity *= STRONG_DAMPING;

			rotation.angularVelocity *= (1.0f - deltaTime.dtSec);

			if (std::abs(rotation.angularVelocity) < STOP_THRESHOLD)
			{
				rotation.angularVelocity = 0.0f;
			}
		}
		else
		{
			rotation.angularVelocity *= dampRotate;
		}
		rotation.angle += rotation.angularVelocity * deltaTime.dtSec;

		float w = 0.0f;
		input.immediateInput.GetState(G_KEY_W, w);

		if (w > 0.0f)
		{
			GW::MATH::GVECTORF thrustDir =
			{
				-std::sin(rotation.angle),  // X 
				0.0f,                      // Y 
				-std::cos(rotation.angle)   // Z 
			};


			GW::MATH::GVector::NormalizeF(thrustDir, thrustDir);
			GW::MATH::GVector::ScaleF(thrustDir, physics.thrust * deltaTime.dtSec, thrustDir);
			GW::MATH::GVector::AddVectorF(physics.velocity, thrustDir, physics.velocity);

			float currentSpeed;
			GW::MATH::GVector::MagnitudeF(physics.velocity, currentSpeed);
			if (currentSpeed > physics.maxSpeed)
			{
				GW::MATH::GVector::NormalizeF(physics.velocity, physics.velocity);
				GW::MATH::GVector::ScaleF(physics.velocity, physics.maxSpeed, physics.velocity);
			}
		}

		physics.velocity.x *= physics.drag;
		physics.velocity.y *= physics.drag;
		physics.velocity.z *= physics.drag;

		float speed;
		GW::MATH::GVector::MagnitudeF(physics.velocity, speed);
		if (speed < 0.01f)
		{
			physics.velocity = { 0.0f, 0.0f, 0.0f };
		}

		GW::MATH::GMatrix::TranslateLocalF(transform, physics.velocity, transform);

		float screenWidth = (*config).at("Screen").at("width").as<float>();
		float screenHeight = (*config).at("Screen").at("height").as<float>();


		float posX = transform.row4.x;
		float posZ = transform.row4.z;

		if (posX > screenWidth / 2) {
			transform.row4.x = -screenWidth / 2;
		}
		else if (posX < -screenWidth / 2) {
			transform.row4.x = screenWidth / 2;
		}

		if (posZ > screenHeight / 2) {
			transform.row4.z = -screenHeight / 2;
		}
		else if (posZ < -screenHeight / 2) {
			transform.row4.z = screenHeight / 2;
		}



		GW::MATH::GMATRIXF rotationMatrix;
		GW::MATH::GMatrix::RotateYGlobalF(transform, rotation.angle, rotationMatrix);
		transform = rotationMatrix;



		if (registry.all_of<FiringState>(view))
		{
			auto& firingState = registry.get<FiringState>(view);
			firingState.fireCoolDown -= deltaTime.dtSec;
			if (firingState.fireCoolDown <= 0.0f)
			{
				registry.remove<FiringState>(view);
			}
		}
		else if (!registry.all_of<FiringState>(view))
		{
			float fire = 0.0f;
			input.immediateInput.GetState(G_KEY_SPACE, fire);

			if (fire > 0.0f)
			{

				auto bullet = registry.create();
				registry.emplace<Projectile>(bullet);
				registry.emplace<Collidable>(bullet);
				auto& bulletMeshes = registry.emplace<DRAW::MeshCollection>(bullet);
				auto& bulletTransform = registry.emplace<Transform>(bullet);
				float x = 0.0f, z = 0.0f;

				GW::MATH::GVECTORF bulletDir =
				{
					-transform.row3.x,
					-transform.row3.y,
					-transform.row3.z
				};

				GW::MATH::GVector::NormalizeF(bulletDir, bulletDir);

				auto& bulletVelocity = registry.emplace<Velocity>(bullet, Velocity{ {bulletDir.x, 0.0f, bulletDir.z} });
				GW::MATH::GVector::NormalizeF(bulletVelocity.velocity, bulletVelocity.velocity);
				GW::MATH::GVector::ScaleF(bulletVelocity.velocity, config.get()->at("Bullet").at("speed").as<float>(), bulletVelocity.velocity);

				UTIL::CreateDynamicObjects(registry, config.get()->at("Bullet").at("model").as<std::string>(), bulletMeshes, bulletTransform);

				bulletTransform.transform = transform;

				registry.emplace<FiringState>(view, FiringState{ config.get()->at("Player").at("firerate").as<float>() });

				auto* sound = registry.try_get<PewPew>(view);
				if (sound != nullptr)
				{
					sound->pewPew.Play();
				}
			}
		}

		if (registry.all_of<Invulnerable>(view))
		{
			auto& invulnerable = registry.get<Invulnerable>(view);
			invulnerable.invulnerableTime -= deltaTime.dtSec;
			if (invulnerable.invulnerableTime <= 0.0f)
			{
				registry.remove<Invulnerable>(view);
			}
		}
	}

	CONNECT_COMPONENT_LOGIC()
	{
		registry.on_update<Player>().connect<UpdatePlayer>();
	}
}