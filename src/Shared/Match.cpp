// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Match.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/Player.hpp>
#include <Shared/Terrain.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <cassert>

// #fixme : Cleanup
#include <NDK/Components.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>

namespace bw
{
	Match::Match(std::string matchName, std::size_t maxPlayerCount) :
	m_sessions(*this),
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName)),
	m_entityStore(true, m_luaInstance)
	{
		MapData mapData;
		mapData.backgroundColor = Nz::Color::Cyan;
		mapData.tileSize = 64.f;

		auto& layer = mapData.layers.emplace_back();
		layer.width = 20;
		layer.height = 10;
		layer.tiles = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 1, 0, 2, 2,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1,
		};

		m_terrain = std::make_unique<Terrain>(std::move(mapData));

		m_luaInstance.LoadLibraries();
		m_entityStore.Load("../../scripts/entities");

		m_entityStore.ForEachEntity([&](const EntityStore::Entity& entity)
		{
			if (entity.isNetworked)
				m_networkStringStore.RegisterString(entity.name);
		});


		std::string entityClassName = "box";

		std::size_t entityIndex = m_entityStore.GetEntityIndex(entityClassName);
		if (entityIndex == EntityStore::InvalidIndex)
		{
			std::cerr << "Entity class \"" << entityClassName << "\" is not registered" << std::endl;
			return;
		}

		auto& entityClass = m_entityStore.GetEntity(entityIndex);

		std::string spritePath;
		bool canRotate;
		float mass;
		float scale;
		unsigned int collisionId;
		try
		{
			m_luaInstance.PushReference(entityClass.tableRef);
			Nz::CallOnExit popOnExit([&] { m_luaInstance.Pop(); });

			canRotate = m_luaInstance.CheckField<bool>("RotationEnabled");
			collisionId = m_luaInstance.CheckField<unsigned int>("CollisionType");
			mass = m_luaInstance.CheckField<float>("Mass");
			scale = m_luaInstance.CheckField<float>("Scale");
			spritePath = m_luaInstance.CheckField<std::string>("Sprite");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClassName << "\" informations: " << e.what() << std::endl;
			return;
		}


		Nz::ImageRef boxImage = Nz::ImageManager::Get(spritePath);
		Nz::Vector2f imageSize = Nz::Vector2f(Nz::Vector3f(boxImage->GetSize())) * scale;

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(-imageSize.x / 2.f, -imageSize.y / 2.f, imageSize.x, imageSize.y));
		burgerBox->SetCollisionId(collisionId);

		const Ndk::EntityHandle& box = m_terrain->GetLayer(0).GetWorld().CreateEntity();
		box->AddComponent<NetworkSyncComponent>("box");
		box->AddComponent<PlayerControlledComponent>();
		box->AddComponent<PlayerMovementComponent>();
		box->AddComponent<Ndk::NodeComponent>().SetPosition(100.f, 100.f);
		box->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = box->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(mass);
		burgerPhys.SetFriction(10.f);

		if (!canRotate)
			burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());
	}

	Match::~Match() = default;

	void Match::Leave(Player* player)
	{
		assert(player->GetMatch() == this);

		auto it = std::find(m_players.begin(), m_players.end(), player);
		assert(it != m_players.end());

		m_players.erase(it);

		player->UpdateLayer(std::numeric_limits<std::size_t>::max());
		player->UpdateMatch(nullptr);
	}

	bool Match::Join(Player* player)
	{
		assert(!player->IsInMatch());

		if (m_players.size() >= m_maxPlayerCount)
			return false;

		m_players.emplace_back(player);
		player->UpdateMatch(this);

		player->UpdateLayer(0);
		player->CreateEntity(m_terrain->GetLayer(0).GetWorld());

		return true;
	}

	void Match::Update(float elapsedTime)
	{
		m_sessions.Poll();
		m_terrain->Update(elapsedTime);

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			session->GetVisibility().UpdateLayer(0); //< HAAAAAX
			session->Update(elapsedTime);
		});
	}
}
