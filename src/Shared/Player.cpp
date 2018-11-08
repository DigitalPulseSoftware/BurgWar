// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Player.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Physics2D/Collider2D.hpp>
#include <NDK/Components.hpp>
#include <Shared/Match.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>

namespace bw
{
	Player::Player(MatchClientSession& session, std::string playerName) :
	m_layerIndex(std::numeric_limits<std::size_t>::max()),
	m_name(std::move(playerName)),
	m_session(session)
	{
	}

	Player::~Player()
	{
		if (m_match)
			m_match->Leave(this);
	}

	const Ndk::EntityHandle& Player::CreateEntity(Ndk::World& world)
	{
		std::string entityClassName = "burger";

		std::size_t entityIndex = m_match->GetEntityStore().GetEntityIndex(entityClassName);
		if (entityIndex == EntityStore::InvalidIndex)
		{
			std::cerr << "Entity class \"" << entityClassName << "\" is not registered" << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}

		auto& entityClass = m_match->GetEntityStore().GetEntity(entityIndex);

		std::string spritePath;
		bool canRotate;
		float mass;
		float scale;
		unsigned int collisionId;
		try
		{
			m_match->GetLuaInstance().PushReference(entityClass.tableRef);
			Nz::CallOnExit popOnExit([&] { m_match->GetLuaInstance().Pop(); });

			canRotate = m_match->GetLuaInstance().CheckField<bool>("RotationEnabled");
			collisionId = m_match->GetLuaInstance().CheckField<unsigned int>("CollisionType");
			mass = m_match->GetLuaInstance().CheckField<float>("Mass");
			scale = m_match->GetLuaInstance().CheckField<float>("Scale");
			spritePath = m_match->GetLuaInstance().CheckField<std::string>("Sprite");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClassName << "\" informations: " << e.what() << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}


		Nz::ImageRef burgerImage = Nz::ImageManager::Get(spritePath);
		Nz::Vector2f burgerSize = Nz::Vector2f(Nz::Vector3f(burgerImage->GetSize())) * scale;

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(collisionId);

		static unsigned int huglyCount = 0;

		m_playerEntity = world.CreateEntity();
		m_playerEntity->AddComponent<NetworkSyncComponent>("burger");
		m_playerEntity->AddComponent<PlayerControlledComponent>();
		m_playerEntity->AddComponent<PlayerMovementComponent>();
		m_playerEntity->AddComponent<Ndk::NodeComponent>().SetPosition(0.f + (huglyCount++) * 200.f, 100.f);
		m_playerEntity->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = m_playerEntity->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(mass);
		burgerPhys.SetFriction(10.f);
		if (!canRotate)
			burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());

		return m_playerEntity;
	}

	void Player::UpdateInput(bool isJumping, bool isMovingLeft, bool isMovingRight)
	{
		if (m_playerEntity)
		{
			assert(m_playerEntity->HasComponent<PlayerControlledComponent>());

			auto& playerController = m_playerEntity->GetComponent<PlayerControlledComponent>();
			playerController.UpdateJumpingState(isJumping);
			playerController.UpdateMovingLeftState(isMovingLeft);
			playerController.UpdateMovingRightState(isMovingRight);
		}
	}

	void Player::UpdateLayer(std::size_t layerIndex)
	{
		m_layerIndex = layerIndex;
	}

	void Player::UpdateMatch(Match* match)
	{
		m_match = match;
	}
}
