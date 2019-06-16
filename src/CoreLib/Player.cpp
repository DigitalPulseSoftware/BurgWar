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
#include <CoreLib/Terrain.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
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
	m_inputIndex(0),
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

	bool Player::GiveWeapon(std::string weaponClass)
	{
		if (!m_match)
			return false;

		if (!m_playerEntity)
			return false;

		if (HasWeapon(weaponClass))
			return false;

		Terrain& terrain = m_match->GetTerrain();
		Ndk::World& world = terrain.GetLayer(m_layerIndex).GetWorld();

		ServerWeaponStore& weaponStore = m_match->GetWeaponStore();

		// Create weapon
		if (std::size_t weaponIndex = weaponStore.GetElementIndex(weaponClass); weaponIndex != ServerEntityStore::InvalidIndex)
		{
			const Ndk::EntityHandle& weapon = weaponStore.InstantiateWeapon(world, weaponIndex, {}, m_playerEntity);
			if (!weapon)
				return false;

			m_weapons.emplace(std::move(weaponClass), weapon);
		}

		return true;
	}

	void Player::Spawn()
	{
		if (!m_match)
			return;

		Terrain& terrain = m_match->GetTerrain();
		Ndk::World& world = terrain.GetLayer(m_layerIndex).GetWorld();

		ServerEntityStore& entityStore = m_match->GetEntityStore();
		if (std::size_t entityIndex = entityStore.GetElementIndex("entity_burger"); entityIndex != ServerEntityStore::InvalidIndex)
		{
			Nz::Vector2f spawnPosition = m_match->GetGamemode()->ExecuteCallback("ChoosePlayerSpawnPosition").as<Nz::Vector2f>();
			const Ndk::EntityHandle& playerEntity = entityStore.InstantiateEntity(world, entityIndex, spawnPosition, 0.f, {});
			if (!playerEntity)
				return;

			std::cout << "[Server] Creating player entity #" << playerEntity->GetId() << std::endl;

			playerEntity->AddComponent<InputComponent>();
			playerEntity->AddComponent<PlayerControlledComponent>(CreateHandle());

			if (playerEntity->HasComponent<HealthComponent>())
			{
				auto& healthComponent = playerEntity->GetComponent<HealthComponent>();

				healthComponent.OnDied.Connect([ply = CreateHandle()](const HealthComponent* health, const Ndk::EntityHandle& attacker)
				{
					if (!ply)
						return;

					ply->GetMatch()->GetGamemode()->ExecuteCallback("OnPlayerDeath", ply->CreateHandle(), attacker);
				});
			}

			UpdateControlledEntity(playerEntity);

			if (!GiveWeapon("weapon_sword_emmentalibur"))
				std::cout << "Failed to give weapon" << std::endl;

			GiveWeapon("weapon_rifle");
		}
	}

	std::string Player::ToString() const
	{
		return "Player(" + m_name + ")";
	}

	void Player::OnTick()
	{
		if (auto& inputOpt = m_inputBuffer[m_inputIndex])
		{
			UpdateInputs(inputOpt.value());
			inputOpt.reset();
		}

		if (++m_inputIndex >= m_inputBuffer.size())
			m_inputIndex = 0;
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
		}
	}

	void Player::UpdateInputs(std::size_t tickDelay, InputData inputData)
	{
		assert(tickDelay < m_inputBuffer.size());
		std::size_t index = (m_inputIndex + tickDelay) % m_inputBuffer.size();

		m_inputBuffer[index] = std::move(inputData);
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
