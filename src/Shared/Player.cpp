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
		Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap("../resources/burger.png");
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Nearest);

		Nz::Vector2f burgerSize = Nz::Vector2f(Nz::Vector3f(burgerMat->GetDiffuseMap()->GetSize())) / 2.f;

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		static unsigned int huglyCount = 0;

		m_playerEntity = world.CreateEntity();
		m_playerEntity->AddComponent<NetworkSyncComponent>();
		m_playerEntity->AddComponent<PlayerControlledComponent>();
		m_playerEntity->AddComponent<PlayerMovementComponent>();
		m_playerEntity->AddComponent<Ndk::NodeComponent>().SetPosition(480.f + (huglyCount++) * 100.f, 100.f);
		m_playerEntity->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = m_playerEntity->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
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
