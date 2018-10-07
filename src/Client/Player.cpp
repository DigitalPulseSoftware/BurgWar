// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Player.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Physics2D/Collider2D.hpp>
#include <NDK/Components.hpp>
#include <Client/Match.hpp>
#include <Client/Components/PlayerControlledComponent.hpp>

namespace bw
{
	Player::Player(std::string playerName) :
	m_layerIndex(std::numeric_limits<std::size_t>::max()),
	m_name(std::move(playerName)),
	m_match(nullptr)
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

		Nz::SpriteRef burgersprite = Nz::Sprite::New();
		burgersprite->SetMaterial(burgerMat);
		burgersprite->SetSize(burgersprite->GetSize() / 2.f);
		Nz::Vector2f burgerSize = burgersprite->GetSize();

		burgersprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(0.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		m_playerEntity = world.CreateEntity();
		m_playerEntity->AddComponent<PlayerControlledComponent>();
		m_playerEntity->AddComponent<Ndk::GraphicsComponent>().Attach(burgersprite);
		m_playerEntity->AddComponent<Ndk::NodeComponent>().SetPosition(300.f, 100.f);
		m_playerEntity->AddComponent<Ndk::CollisionComponent2D>(burgerBox);

		auto& burgerPhys = m_playerEntity->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
		burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());

		/*Nz::MaterialRef weaponMat = Nz::Material::New("Translucent2D");
		weaponMat->SetDiffuseMap("../resources/emmentalibur.png");
		auto& sampler2 = weaponMat->GetDiffuseSampler();
		sampler2.SetFilterMode(Nz::SamplerFilter_Nearest);

		Nz::SpriteRef weaponSprite = Nz::Sprite::New();
		weaponSprite->SetMaterial(weaponMat);
		weaponSprite->SetSize(weaponSprite->GetSize() / 1.4f);
		weaponSprite->SetOrigin(Nz::Vector2f(40.f, 284.f) / 1.4f);

		const Ndk::EntityHandle& weapon = world.CreateEntity();
		weapon->AddComponent<Ndk::GraphicsComponent>().Attach(weaponSprite, -1.f);
		auto& weaponNode = weapon->AddComponent<Ndk::NodeComponent>();
		weaponNode.SetParent(burger);
		weaponNode.SetPosition(30.f, -80.f);*/

		return m_playerEntity;
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
