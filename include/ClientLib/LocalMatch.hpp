// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALMATCH_HPP
#define BURGWAR_CLIENTLIB_LOCALMATCH_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <CoreLib/AnimationManager.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <tsl/hopscotch_map.h>
#include <Nazara/Platform/EventHandler.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace bw
{
	class BurgApp;
	class ClientGamemode;
	class InputController;
	class VirtualDirectory;

	class LocalMatch : public SharedMatch, public std::enable_shared_from_this<LocalMatch>
	{
		friend class ClientSession;

		public:
			LocalMatch(BurgApp& burgApp, Nz::RenderTarget* renderTarget, ClientSession& session, const Packets::MatchData& matchData, std::shared_ptr<InputController> inputController);
			~LocalMatch() = default;

			void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) override;

			inline AnimationManager& GetAnimationManager();
			inline BurgApp& GetApplication();
			inline const Ndk::EntityHandle& GetCamera();
			inline const Nz::SpriteRef& GetTrailSprite() const;

			void LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir);

			void Update(float elapsedTime);

		private:
			struct ServerEntity;

			void ControlEntity(Nz::UInt8 playerIndex, Nz::UInt32 serverId);
			Ndk::EntityHandle CreateEntity(Nz::UInt32 serverId, const std::string& entityClassName, const Nz::Vector2f& createPosition, bool hasPlayerMovement, bool hasInputs, bool isPhysical, std::optional<Nz::UInt32> parentId, Nz::UInt16 currentHealth, Nz::UInt16 maxHealth, const EntityProperties& properties, const std::string& name);

			void CreateHealthBar(ServerEntity& serverEntity, Nz::UInt16 currentHealth);
			void CreateName(ServerEntity& serverEntity, const std::string& name);
			void DebugEntityId(ServerEntity& serverEntity);
			void DeleteEntity(Nz::UInt32 serverId);
			void MoveEntity(Nz::UInt32 serverId, const Nz::Vector2f& newPos, const Nz::Vector2f& newLinearVel, Nz::RadianAnglef newRot, Nz::RadianAnglef newAngularVel, bool isFacingRight);
			void PlayAnimation(Nz::UInt32 serverId, Nz::UInt8 animId);
			void SendInputs();
			void UpdateEntityHealth(Nz::UInt32 serverId, Nz::UInt16 newHealth);
			void UpdateEntityInput(Nz::UInt32 serverId, const InputData& inputs);

			struct HealthData
			{
				float spriteWidth;
				Nz::UInt16 currentHealth;
				Nz::SpriteRef healthSprite;
				Ndk::EntityOwner healthBarEntity;
			};

			struct NameData
			{
				Ndk::EntityOwner nameEntity;
			};

			struct PlayerData
			{
				PlayerData(Nz::UInt8 playerIndex) :
				playerIndex(playerIndex)
				{
				}

				Ndk::EntityHandle controlledEntity;
				Nz::UInt8 playerIndex;
				InputData lastInputData;
			};

			struct ServerEntity
			{
				std::optional<HealthData> health;
				std::optional<NameData> name;
				Ndk::EntityOwner entity;
				Nz::RadianAnglef rotationError = 0.f;
				Nz::Vector2f positionError = Nz::Vector2f::Zero();
				Nz::UInt16 maxHealth;
				Nz::UInt32 serverEntityId;
				Nz::UInt32 weaponEntityId = 0xFFFFFFFF;
				bool isPhysical;
			};

			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::EventHandler, OnKeyReleased, m_onKeyReleasedSlot);

			std::optional<ClientEntityStore> m_entityStore;
			std::optional<ClientWeaponStore> m_weaponStore;
			std::shared_ptr<ClientGamemode> m_gamemode;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<InputController> m_inputController;
			std::string m_gamemodePath;
			std::vector<PlayerData> m_playerData;
			tsl::hopscotch_map<Nz::UInt32 /*serverEntityId*/, ServerEntity /*clientEntity*/> m_serverEntityIdToClient;
			Ndk::EntityHandle m_camera;
			Ndk::World m_world;
			Nz::SpriteRef m_trailSpriteTest;
			AnimationManager m_animationManager;
			BurgApp& m_application;
			ClientSession& m_session;
			Packets::PlayersInput m_inputPacket;
			float m_errorCorrectionTimer;
			float m_playerEntitiesTimer;
			float m_playerInputTimer;
	};
}

#include <ClientLib/LocalMatch.inl>

#endif