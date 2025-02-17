#include "CCL.h"
#include "DRAW/DrawComponents.h"
#include "GAME/GameComponents.h"

namespace DRAW 
{
	void Construct_CPULevel(entt::registry& registry, entt::entity entity)
	{
		auto& level = registry.get<CPULevel>(entity);
		GW::SYSTEM::GLog levelLog;
		levelLog.Create("LevelLog");

		bool loaded = level.levelData.LoadLevel(level.levelFilePath.c_str(), level.levelModelPath.c_str(), levelLog);
	}

    void Construct_GPULevel(entt::registry& registry, entt::entity entity)
    {
        if (!registry.all_of<DRAW::CPULevel>(entity)) 
        {
            return;
        }

        auto view = registry.view<GeometryData, GPUInstance>();
        for (auto entity : view) 
        {
            registry.destroy(entity);
        }

        if (registry.all_of<DRAW::GPULevel>(entity)) 
        {
            registry.replace<DRAW::GPULevel>(entity);
        }
        else 
        {
            registry.emplace<DRAW::GPULevel>(entity);
        }

        auto& level = registry.get<DRAW::CPULevel>(entity);

        if (registry.all_of<DRAW::VulkanIndexBuffer>(entity)) 
        {
            registry.remove<DRAW::VulkanIndexBuffer>(entity);
        }
        if (registry.all_of<DRAW::VulkanVertexBuffer>(entity)) 
        {
            registry.remove<DRAW::VulkanVertexBuffer>(entity);
        }

        registry.emplace<DRAW::VulkanIndexBuffer>(entity);
        registry.emplace<std::vector<unsigned int>>(entity, level.levelData.levelIndices);
        registry.patch<DRAW::VulkanIndexBuffer>(entity);

        registry.emplace<DRAW::VulkanVertexBuffer>(entity);
        registry.emplace<std::vector<H2B::VERTEX>>(entity, level.levelData.levelVertices);
        registry.patch<DRAW::VulkanVertexBuffer>(entity);

        auto& modelManager = registry.ctx().emplace<ModelManager>();

        for (auto& var : level.levelData.blenderObjects)
        {
            MeshCollection objectMeshes;
            for (size_t i = 0; i < level.levelData.levelModels[var.modelIndex].meshCount; i++)
            {
                auto meshEntity = registry.create();
                registry.emplace<GeometryData>(meshEntity, GeometryData
                    {
                    level.levelData.levelMeshes[level.levelData.levelModels[var.modelIndex].meshStart + i].drawInfo.indexOffset +
                    level.levelData.levelModels[var.modelIndex].indexStart,
                    level.levelData.levelMeshes[level.levelData.levelModels[var.modelIndex].meshStart + i].drawInfo.indexCount,
                    level.levelData.levelModels[var.modelIndex].vertexStart
                    });

                registry.emplace<GPUInstance>(meshEntity, GPUInstance
                    {
                    level.levelData.levelTransforms[var.transformIndex],
                    level.levelData.levelMaterials[level.levelData.levelModels[var.modelIndex].materialStart + i].attrib
                    });

                if (level.levelData.levelModels[var.modelIndex].isDynamic) 
                {
                    registry.emplace<DoNotRender>(meshEntity);
                    objectMeshes.meshes.push_back(meshEntity);
                }

                objectMeshes.boundingBox = level.levelData.levelColliders[level.levelData.levelModels[var.modelIndex].colliderIndex];

                if (level.levelData.levelModels[var.modelIndex].isCollidable) 
                {
                    auto collisionEntity = registry.create();
                    registry.emplace<GAME::Collidable>(collisionEntity);
                    registry.emplace<GAME::Obstacle>(collisionEntity);
                    registry.emplace<MeshCollection>(collisionEntity, objectMeshes);
                    registry.emplace<GAME::Transform>(collisionEntity, GAME::Transform{ level.levelData.levelTransforms[var.transformIndex] });
                }
            }

            if (level.levelData.levelModels[var.modelIndex].isDynamic) 
            {
                modelManager.models.emplace(var.blendername, objectMeshes);
            }
        }
    }

	void Destroy_MeshCollection(entt::registry& registry, entt::entity entity)
	{
		auto& meshCollection = registry.get<MeshCollection>(entity);
		for each (auto var in meshCollection.meshes)
		{
            if(registry.valid(var))
                registry.destroy(var);
		}
	}	
	
	CONNECT_COMPONENT_LOGIC() {
		registry.on_construct<CPULevel>().connect<Construct_CPULevel>();
		registry.on_construct<GPULevel>().connect<Construct_GPULevel>();

		registry.on_destroy<MeshCollection>().connect<Destroy_MeshCollection>();
	}
}