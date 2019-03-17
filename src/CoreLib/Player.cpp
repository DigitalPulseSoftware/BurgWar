// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Player.hpp>
#include <Nazara/Core/CallOnExit.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Physics2D/Collider2D.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>

namespace bw
{
	Player::Player(MatchClientSession& session, Nz::UInt8 playerIndex, std::string playerName) :
	m_layerIndex(std::numeric_limits<std::size_t>::max()),
	m_name(std::move(playerName)),
	m_playerIndex(playerIndex),
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
		ServerEntityStore& entityStore = m_match->GetEntityStore();
		ServerWeaponStore& weaponStore = m_match->GetWeaponStore();
		if (std::size_t entityIndex = entityStore.GetElementIndex("entity_burger"); entityIndex != ServerEntityStore::InvalidIndex)
		{
			static unsigned int huglyCount = 0;
			const Ndk::EntityHandle& burger = entityStore.InstantiateEntity(world, entityIndex, { 200.f + (huglyCount++) * 100.f, 100.f }, 0.f, {});
			if (!burger)
				return Ndk::EntityHandle::InvalidHandle;

			burger->AddComponent<InputComponent>();
			burger->AddComponent<PlayerControlledComponent>(CreateHandle());

			if (burger->HasComponent<HealthComponent>())
			{
				auto& healthComponent = burger->GetComponent<HealthComponent>();

				healthComponent.OnDied.Connect([ply = CreateHandle()](const HealthComponent* health, const Ndk::EntityHandle& attacker)
				{
					if (!ply)
						return;

					/*ply->GetMatch()->GetGamemode()->ExecuteCallback("OnPlayerDeath", [&](Nz::LuaState& state)
					{
						state.PushLightUserdata(ply.GetObject());
						state.Push(attacker);
						return 2;
					});*/
				});
			}

			UpdateControlledEntity(burger);

			// Create weapon
			if (std::size_t weaponIndex = weaponStore.GetElementIndex("weapon_sword_emmentalibur"); weaponIndex != ServerEntityStore::InvalidIndex)
				m_playerWeapon = weaponStore.InstantiateWeapon(world, weaponIndex, {}, burger);

			return burger;
		}

		return Ndk::EntityHandle::InvalidHandle;
	}

	void Player::UpdateControlledEntity(const Ndk::EntityHandle& entity)
	{
		m_playerEntity = entity;

		Packets::ControlEntity controlEntity;
		controlEntity.entityId = (entity) ? static_cast<Nz::UInt32>(entity->GetId()) : 0;
		controlEntity.playerIndex = m_playerIndex;

		m_session.GetVisibility().SendEntityPacket(controlEntity.entityId, controlEntity);
	}

	void Player::UpdateInputs(const InputData& inputData)
	{
		if (m_playerEntity)
		{
			assert(m_playerEntity->HasComponent<InputComponent>());

			auto& inputComponent = m_playerEntity->GetComponent<InputComponent>();
			inputComponent.UpdateInputs(inputData);

			if (m_playerWeapon)
			{
				auto& weaponNode = m_playerWeapon->GetComponent<Ndk::NodeComponent>();

				if (inputData.isAttacking)
				{
					auto& weaponScript = m_playerWeapon->GetComponent<ScriptComponent>();
					weaponScript.ExecuteCallback("OnAttack", weaponScript.GetTable());
				}

				Nz::RadianAnglef angle(std::atan2(inputData.aimDirection.y, inputData.aimDirection.x));
				if (weaponNode.GetScale().x < 0.f)
					angle += Nz::RadianAnglef(float(M_PI));

				weaponNode.SetRotation(angle);

				if (m_playerWeapon->HasComponent<NetworkSyncComponent>())
					m_playerWeapon->GetComponent<NetworkSyncComponent>().Invalidate();
			}
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
