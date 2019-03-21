// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/Utility/TileMapData.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <NDK/Components.hpp>
#include <iostream>

namespace bw
{
	const Ndk::EntityHandle& ClientEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const
	{
		const auto& entityClass = GetElement(entityIndex);

		std::string spritePath;
		bool hasInputs;
		bool playerControlled;
		float scale;
		try
		{
			hasInputs = entityClass->elementTable["HasInputs"];
			playerControlled = entityClass->elementTable["PlayerControlled"];
			scale = entityClass->elementTable["Scale"];
			spritePath = entityClass->elementTable["Sprite"];
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClass->name << "\" informations: " << e.what() << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}

		const Ndk::EntityHandle& entity = CreateEntity(world, entityClass, properties);

		entity->AddComponent<Ndk::GraphicsComponent>();

		// Warning what's following is ugly
		if (entityClass->name == "burger")
		{
			Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
			mat->SetDiffuseMap(spritePath);
			auto& sampler = mat->GetDiffuseSampler();
			sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

			Nz::SpriteRef sprite = Nz::Sprite::New();
			sprite->SetMaterial(mat);
			sprite->SetSize(sprite->GetSize() * scale);
			Nz::Vector2f burgerSize = sprite->GetSize();

			sprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));

			entity->GetComponent<Ndk::GraphicsComponent>().Attach(sprite);
		}

		auto& nodeComponent = entity->AddComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (hasInputs)
			entity->AddComponent<InputComponent>();

		if (!InitializeEntity(*entityClass, entity))
			entity->Kill();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			entity->GetComponent<Ndk::PhysicsComponent2D>().EnableNodeSynchronization(false);

		return entity;
	}

	void ClientEntityStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedEntityStore::InitializeElementTable(elementTable);
		
		elementTable["AddSprite"] = [](const sol::table& entityTable, const std::string& texturePath, const Nz::Vector2f& scale)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];

			Nz::MaterialRef mat = Nz::Material::New("Translucent2D");
			mat->SetDiffuseMap(texturePath);
			auto& sampler = mat->GetDiffuseSampler();
			sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

			Nz::SpriteRef sprite = Nz::Sprite::New();
			sprite->SetMaterial(mat);
			sprite->SetSize(sprite->GetSize() * scale);
			Nz::Vector2f burgerSize = sprite->GetSize();

			sprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y / 2.f));

			Ndk::GraphicsComponent& gfxComponent = (entity->HasComponent<Ndk::GraphicsComponent>()) ? entity->GetComponent<Ndk::GraphicsComponent>() : entity->AddComponent<Ndk::GraphicsComponent>();
			gfxComponent.Attach(sprite);
		};

		elementTable["AddTilemap"] = [](const sol::table& entityTable, const Nz::Vector2ui& mapSize, const Nz::Vector2f& cellSize, const sol::table& content, const std::vector<TileData>& tiles)
		{
			const Ndk::EntityHandle& entity = entityTable["Entity"];

			// Compute tilemap
			tsl::hopscotch_map<Nz::MaterialRef /*material*/, std::size_t /*materialIndex*/> materials;
			for (const auto& tile : tiles)
			{
				auto it = materials.find(tile.material);
				if (it == materials.end())
					materials.emplace(tile.material, materials.size());
			}

			Nz::TileMapRef tileMap = Nz::TileMap::New(mapSize, cellSize, materials.size());
			for (auto&& [material, matIndex] : materials)
				tileMap->SetMaterial(matIndex, material);

			std::size_t cellCount = content.size();
			std::size_t expectedCellCount = mapSize.x * mapSize.y;
			if (cellCount != expectedCellCount)
			{
				std::cerr << "Expected " << expectedCellCount << " cells, got " << cellCount << std::endl;
				cellCount = std::min(cellCount, expectedCellCount);
			}

			for (std::size_t i = 0; i < cellCount; ++i)
			{
				unsigned int value = content[i + 1];

				Nz::Vector2ui tilePos = { static_cast<unsigned int>(i % mapSize.x), static_cast<unsigned int>(i / mapSize.x) };
				if (value > 0)
				{
					if (value - 1 >= content.size())
						throw std::runtime_error("");

					const auto& tileData = tiles[value - 1];

					auto matIt = materials.find(tileData.material);
					assert(matIt != materials.end());

					tileMap->EnableTile(tilePos, tileData.texCoords, Nz::Color::White, matIt->second);
				}
			}

			Ndk::GraphicsComponent& gfxComponent = (entity->HasComponent<Ndk::GraphicsComponent>()) ? entity->GetComponent<Ndk::GraphicsComponent>() : entity->AddComponent<Ndk::GraphicsComponent>();
			gfxComponent.Attach(tileMap);
		};
	}

	void ClientEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& entity)
	{
		SharedEntityStore::InitializeElement(elementTable, entity);
	}
}
