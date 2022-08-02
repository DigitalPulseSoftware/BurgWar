// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Player.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/Map.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/MatchClientVisibility.hpp>
#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Terrain.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <Nazara/Physics2D/Collider2D.hpp>

namespace bw
{
	Player::Player(Match& match, MatchClientSession& session, std::size_t playerIndex, Nz::UInt8 localIndex, std::string playerName) :
	m_layerIndex(NoLayer),
	m_playerIndex(playerIndex),
	m_name(std::move(playerName)),
	m_localIndex(localIndex),
	m_match(match),
	m_session(session),
	m_isAdmin(false),
	m_isReady(false),
	m_shouldSendWeapons(false)
	{
	}

	Player::~Player()
	{
		MatchClientVisibility& visibility = GetSession().GetVisibility();
		for (std::size_t layerIndex = m_visibleLayers.FindFirst(); layerIndex != m_visibleLayers.npos; layerIndex = m_visibleLayers.FindNext(layerIndex))
			visibility.HideLayer(static_cast<LayerIndex>(layerIndex));
	}

	void Player::HandleConsoleCommand(const std::string& str)
	{
		if (!m_isAdmin)
			return;

		if (!m_scriptingEnvironment)
		{
			const std::string& scriptFolder = m_match.GetApp().GetConfig().GetStringValue("Resources.ScriptDirectory");

			m_scriptingEnvironment.emplace(m_match.GetLogger(), m_match.GetScriptingLibrary(), std::make_shared<Nz::VirtualDirectory>(scriptFolder));
			m_scriptingEnvironment->SetOutputCallback([ply = CreateHandle()](const std::string& text, Nz::Color color)
			{
				if (!ply)
					return;

				Packets::ConsoleAnswer answer;
				answer.color = color;
				answer.localIndex = ply->GetLocalIndex();
				answer.response = text;

				ply->SendPacket(std::move(answer));
			});
		}

		m_scriptingEnvironment->Execute(str);
	}

	void Player::MoveToLayer(LayerIndex layerIndex)
	{
		if (m_layerIndex != layerIndex)
		{
			m_match.GetGamemode()->ExecuteCallback<GamemodeEvent::PlayerLayerUpdate>(CreateHandle(), m_layerIndex, layerIndex);

			if (m_layerIndex != NoLayer)
				UpdateLayerVisibility(m_layerIndex, false);

			if (m_layerIndex != NoLayer && layerIndex != NoLayer)
			{
				if (m_playerEntity)
				{
					Terrain& terrain = m_match.GetTerrain();
					entt::registry& world = terrain.GetLayer(layerIndex).GetWorld();

#if 0

					entt::entity newPlayerEntity = world.CloneEntity(m_playerEntity);
					/*const auto& componentBits = m_playerEntity->GetComponentBits();
					for (std::size_t i = componentBits.FindFirst(); i != componentBits.npos; i = componentBits.FindNext(i))
					{
						// Leave physics component because dropping it during a physics callback would crash the physics engine
						if (i != Ndk::GetComponentIndex<MatchComponent>() && i != Ndk::GetComponentIndex<Ndk::PhysicsComponent2D>())
							newPlayerEntity->AddComponent(m_playerEntity->DropComponent(static_cast<Ndk::ComponentIndex>(i)));
					}

					newPlayerEntity->AddComponent(m_playerEntity->GetComponent<Ndk::PhysicsComponent2D>().Clone());*/

					EntityId uniqueId = m_match.AllocateUniqueId();

					newPlayerEntity->AddComponent<MatchComponent>(m_match, layerIndex, uniqueId);

					m_match.RegisterEntity(uniqueId, newPlayerEntity);

					UpdateControlledEntity(newPlayerEntity, true, true);

					if (m_playerEntity->HasComponent<WeaponWielderComponent>())
					{
						auto& weaponWielder = m_playerEntity->GetComponent<WeaponWielderComponent>();

						weaponWielder.OverrideEntities([&](Ndk::EntityOwner& weaponEntity)
						{
							EntityId weaponUniqueId = m_match.AllocateUniqueId();

							weaponEntity = world.CloneEntity(weaponEntity);
							weaponEntity->AddComponent<MatchComponent>(m_match, layerIndex, weaponUniqueId);
							weaponentity.get<Nz::NodeComponent>().SetParent(newPlayerEntity);
							weaponEntity->GetComponent<NetworkSyncComponent>().UpdateParent(newPlayerEntity);
							weaponEntity->GetComponent<WeaponComponent>().UpdateOwner(newPlayerEntity);

							m_match.RegisterEntity(weaponUniqueId, weaponEntity);
						});
					}

#endif

					m_shouldSendWeapons = true;
				}
			}
			else
				m_playerEntity.reset();

			m_layerIndex = layerIndex;

			MatchClientVisibility& visibility = GetSession().GetVisibility();
			visibility.PushLayerUpdate(m_localIndex, m_layerIndex);

			if (m_layerIndex != NoLayer)
				UpdateLayerVisibility(m_layerIndex, true);
		}
	}

	void Player::PrintChatMessage(std::string message)
	{
		Packets::ChatMessage chatPacket;
		chatPacket.content = std::move(message);
		chatPacket.localIndex = m_localIndex;

		SendPacket(chatPacket);
	}
	
	void Player::OnTick(bool lastTick)
	{
		if (lastTick && m_shouldSendWeapons)
		{
			Packets::PlayerWeapons weaponPacket;
			weaponPacket.localIndex = m_localIndex;
			weaponPacket.layerIndex = m_layerIndex;

			Nz::Bitset<Nz::UInt64> weaponIds;
			if (m_playerEntity)
			{
				entt::registry* registry = m_playerEntity->GetEntity().registry();
				if (WeaponWielderComponent* weaponWielder = registry->try_get<WeaponWielderComponent>(m_playerEntity->GetEntity()))
				{
					for (entt::handle weapon : weaponWielder->GetWeapons())
					{
						assert(weapon.valid());

						auto& weaponNetwork = weapon.get<NetworkSyncComponent>();

						weaponPacket.weaponEntities.emplace_back(weaponNetwork.GetNetworkId());
						weaponIds.UnboundedSet(weaponNetwork.GetNetworkId());
					}
				}
			}

			m_session.GetVisibility().PushEntitiesPacket(m_layerIndex, std::move(weaponIds), std::move(weaponPacket));

			m_shouldSendWeapons = false;
		}
	}

	void Player::SetAdmin(bool isAdmin)
	{
		m_isAdmin = isAdmin;
	}
	
	std::string Player::ToString() const
	{
		return "Player(" + m_name + ")";
	}

	void Player::UpdateControlledEntity(entt::handle entity, bool sendPacket, bool ignoreLayerUpdate)
	{
		MatchClientVisibility& visibility = m_session.GetVisibility();

		if (m_playerEntity)
		{
			EntityOwner& playerEntity = *m_playerEntity;

			playerEntity->remove<OwnerComponent>();
			playerEntity->remove<PlayerControlledComponent>();

			auto& matchComponent = playerEntity->get<MatchComponent>();
			auto& networkComponent = playerEntity->get<NetworkSyncComponent>();
			visibility.SetEntityControlledStatus(matchComponent.GetLayerIndex(), networkComponent.GetNetworkId(), false);
		}

		m_playerEntity.reset();
		m_onPlayerEntityDie.Disconnect();
		m_onPlayerEntityDestruction.Disconnect();
		m_onWeaponAdded.Disconnect();
		m_onWeaponRemove.Disconnect();

		EntityId entityUniqueId = InvalidEntityId;
		if (entity)
		{
			auto& matchComponent = entity.get<MatchComponent>();
			if (!ignoreLayerUpdate)
				MoveToLayer(matchComponent.GetLayerIndex());

			entityUniqueId = matchComponent.GetUniqueId();

			m_playerEntity = entity; //< FIXME (deferred because of MoveToLayer)

			if (WeaponWielderComponent* weaponWielder = entity.try_get<WeaponWielderComponent>())
			{
				auto onWeaponSetUpdate = [&](WeaponWielderComponent* /*wielder*/, const std::string& /*weaponClass*/, std::size_t /*weaponIndex*/)
				{
					m_shouldSendWeapons = true;
				};

				m_onWeaponAdded.Connect(weaponWielder->OnWeaponAdded, onWeaponSetUpdate);
				m_onWeaponRemove.Connect(weaponWielder->OnWeaponRemove, onWeaponSetUpdate);
			}

			entity.emplace<OwnerComponent>(CreateHandle());
			entity.emplace<PlayerControlledComponent>(CreateHandle());

			if (HealthComponent* healthComponent = entity.try_get<HealthComponent>())
			{
				m_onPlayerEntityDie.Connect(healthComponent->OnDie, [this](const HealthComponent* /*health*/, entt::handle attacker)
				{
					OnDeath(attacker);
				});
			}

			auto& destructionWatcher = entity.get_or_emplace<DestructionWatcherComponent>(entity);
			m_onPlayerEntityDestruction.Connect(destructionWatcher.OnDestruction, [this](DestructionWatcherComponent* /*destructionWatcher*/)
			{
				OnDeath({});
			});

			auto& networkComponent = entity.get<NetworkSyncComponent>();
			visibility.SetEntityControlledStatus(matchComponent.GetLayerIndex(), networkComponent.GetNetworkId(), true);
		}

		if (sendPacket)
		{
			Packets::ControlEntity controlEntity;
			controlEntity.localIndex = m_localIndex;
			if (m_playerEntity)
			{
				EntityOwner& playerEntity = *m_playerEntity;

				auto& matchComponent = playerEntity->get<MatchComponent>();
				auto& networkComponent = playerEntity->get<NetworkSyncComponent>();

				controlEntity.layerIndex = matchComponent.GetLayerIndex();
				controlEntity.entityId = networkComponent.GetNetworkId();

				visibility.PushEntityPacket(matchComponent.GetLayerIndex(), controlEntity.entityId, controlEntity);
			}
			else
			{
				controlEntity.layerIndex = NoLayer;
				controlEntity.entityId = 0;

				SendPacket(controlEntity);
			}
		}

		// Send a packet to everyone to notify of the new controlled entity
		Packets::PlayerControlEntity controlledEntityUpdate;
		controlledEntityUpdate.playerIndex = Nz::UInt16(m_playerIndex);
		controlledEntityUpdate.controlledEntityId = entityUniqueId;

		m_match.BroadcastPacket(controlledEntityUpdate, true, this);
	}

	void Player::UpdateLayerVisibility(LayerIndex layerIndex, bool isVisible)
	{
		MatchClientVisibility& visibility = GetSession().GetVisibility();

		if (layerIndex >= m_match.GetTerrain().GetLayerCount())
			throw std::runtime_error("Layer index out of bounds");

		if (isVisible)
			visibility.ShowLayer(layerIndex);
		else
			visibility.HideLayer(layerIndex);

		m_visibleLayers.UnboundedSet(layerIndex, isVisible);
	}

	void Player::UpdateName(std::string newName)
	{
		m_match.GetGamemode()->ExecuteCallback<GamemodeEvent::PlayerNameUpdate>(CreateHandle(), newName);
		m_name = std::move(newName);

		Packets::PlayerNameUpdate nameUpdatePacket;
		nameUpdatePacket.newName = m_name;
		nameUpdatePacket.playerIndex = Nz::UInt16(m_playerIndex);

		m_match.BroadcastPacket(nameUpdatePacket);
	}

	void Player::OnDeath(entt::handle attacker)
	{
		assert(m_playerEntity);

		UpdateControlledEntity(entt::handle{}, false);

		Packets::ChatMessage chatPacket;
		if (attacker)
		{
			if (OwnerComponent* ownerComponent = attacker.try_get<OwnerComponent>())
			{
				Player* playerKiller = ownerComponent->GetOwner();
				if (playerKiller != this)
					chatPacket.content = playerKiller->GetName() + " killed " + GetName();
				else
					chatPacket.content = GetName() + " suicided";
			}
			else
				chatPacket.content = GetName() + " suicided";
		}
		else
			chatPacket.content = GetName() + " suicided";

		m_match.ForEachPlayer([&](Player* otherPlayer)
		{
			otherPlayer->SendPacket(chatPacket);
		});

		if (attacker)
		{
			if (ScriptComponent* attackerScript = attacker.try_get<ScriptComponent>())
				m_match.GetGamemode()->ExecuteCallback<GamemodeEvent::PlayerDeath>(CreateHandle(), attackerScript->GetTable());
			else
				m_match.GetGamemode()->ExecuteCallback<GamemodeEvent::PlayerDeath>(CreateHandle(), sol::nil);
		}
		else
			m_match.GetGamemode()->ExecuteCallback<GamemodeEvent::PlayerDeath>(CreateHandle(), sol::nil);
	}

	void Player::SetReady()
	{
		assert(!m_isReady);
		m_isReady = true;
	}
}
