// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityLibrary.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Components/VisualInterpolationComponent.hpp>
#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
#include <ClientLib/Scripting/Tilemap.hpp>
#include <ClientLib/Utility/TileMapData.hpp>
#include <Nazara/Graphics/Graphics.hpp>
#include <Nazara/Graphics/MaterialInstance.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/Tilemap.hpp>
#include <Nazara/Graphics/Components/GraphicsComponent.hpp>
#include <Nazara/Math/EulerAngles.hpp>
#include <Nazara/ChipmunkPhysics2D/ChipmunkRigidBody2D.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	void ClientEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedEntityLibrary::RegisterLibrary(elementMetatable);

		RegisterClientLibrary(elementMetatable);
	}

	void ClientEntityLibrary::InitRigidBody(lua_State* L, entt::handle entity, float mass)
	{
		std::shared_ptr<Nz::ChipmunkCollider2D> collider;
		if (auto* rigidBody = entity.try_get<Nz::ChipmunkRigidBody2DComponent>())
		{
			collider = rigidBody->GetGeom();
			entity.erase<Nz::ChipmunkRigidBody2DComponent>();
		}

		Nz::ChipmunkRigidBody2D::DynamicSettings settings;
		settings.geom = collider;
		settings.mass = mass;

		auto& entityPhys = entity.emplace<Nz::ChipmunkRigidBody2DComponent>(settings);

		//entity.get<Nz::ChipmunkRigidBody2DComponent>().EnableNodeSynchronization(false);
		entity.emplace_or_replace<VisualInterpolationComponent>();
	}

	void ClientEntityLibrary::RegisterClientLibrary(sol::table& elementMetatable)
	{
		elementMetatable["AddLayer"] = LuaFunction([](sol::this_state L, const sol::table& entityTable, const sol::table& parameters)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			auto& clientMatch = entity.get<ClientMatchComponent>().GetClientMatch();

			LayerIndex layerIndex = parameters["LayerIndex"];
			if (layerIndex >= clientMatch.GetLayerCount())
				TriggerLuaArgError(L, 2, "layer index out of bounds");

			int renderOrder = parameters.get_or("RenderOrder", 0);
			Nz::Vector2f parallaxFactor = parameters.get_or("ParallaxFactor", Nz::Vector2f::Unit());
			Nz::Vector2f scale = parameters.get_or("Scale", Nz::Vector2f::Unit());

			auto& visibleLayer = entity.get_or_emplace<VisibleLayerComponent>(clientMatch.GetRenderWorld(), entity);
			visibleLayer.RegisterLocalLayer(clientMatch.GetLayer(layerIndex), renderOrder, scale, parallaxFactor);
		});

		elementMetatable["AddTilemap"] = LuaFunction([this](const sol::table& entityTable, const Nz::Vector2ui& mapSize, const Nz::Vector2f& cellSize, const sol::table& content, const std::vector<TileData>& tiles, int renderOrder = 0) -> sol::optional<Tilemap>
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			// Compute tilemap
			tsl::hopscotch_map<std::string /*materialPath*/, std::size_t /*materialIndex*/> materials;
			for (const auto& tile : tiles)
			{
				auto it = materials.find(tile.materialPath);
				if (it == materials.end())
					materials.emplace(tile.materialPath, materials.size());
			}

			if (materials.empty())
				return {};

			std::shared_ptr<Nz::Tilemap> tilemap = std::make_shared<Nz::Tilemap>(mapSize, cellSize, materials.size());
			for (auto&& [materialPath, matIndex] : materials)
			{
				std::shared_ptr<Nz::MaterialInstance> material = Nz::MaterialInstance::Instantiate(Nz::MaterialType::Basic, Nz::MaterialInstancePreset::Transparent);
				material->SetTextureProperty("BaseColorMap", m_assetStore.GetTexture(materialPath));

				tilemap->SetMaterial(matIndex, material);
			}

			tilemap->UpdateRenderLayer(renderOrder);

			std::size_t cellCount = content.size();
			std::size_t expectedCellCount = mapSize.x * mapSize.y;
			if (cellCount != expectedCellCount)
			{
				bwLog(GetLogger(), LogLevel::Warning, "Expected {0} cells, got {1}", expectedCellCount, cellCount);
				cellCount = std::min(cellCount, expectedCellCount);
			}

			for (std::size_t i = 0; i < cellCount; ++i)
			{
				unsigned int value = content[i + 1];

				Nz::Vector2ui tilePos = { static_cast<unsigned int>(i % mapSize.x), static_cast<unsigned int>(i / mapSize.x) };
				if (value > 0)
				{
					if (value <= tiles.size())
					{
						const auto& tileData = tiles[value - 1];

						auto matIt = materials.find(tileData.materialPath);
						assert(matIt != materials.end());

						tilemap->EnableTile(tilePos, tileData.texCoords, Nz::Color::White(), matIt->second);
					}
				}
			}

			Nz::Matrix4f transformMatrix = Nz::Matrix4f::Identity();

			auto& visualComponent = entity.get<VisualComponent>();

			Tilemap scriptTilemap(visualComponent.GetLayerVisual(), std::move(tilemap), Nz::Vector2f::Zero(), Nz::RadianAnglef::Zero());
			scriptTilemap.Show();

			return scriptTilemap;
		});

		elementMetatable["ClearLayers"] = LuaFunction([](const sol::table& entityTable)
		{
			entt::handle entity = AssertScriptEntity(entityTable);

			if (VisibleLayerComponent* visibleLayer = entity.try_get<VisibleLayerComponent>())
				visibleLayer->Clear();
		});
	}
}
