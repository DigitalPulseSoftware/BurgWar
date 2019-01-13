// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALMATCH_HPP
#define BURGWAR_CLIENT_LOCALMATCH_HPP

#include <Client/LocalInputController.hpp>
#include <Client/Scripting/ClientEntityStore.hpp>
#include <Client/Scripting/ClientScriptingContext.hpp>
#include <Client/Scripting/ClientWeaponStore.hpp>
#include <Shared/AnimationManager.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <hopscotch/hopscotch_map.h>
#include <Nazara/Platform/EventHandler.hpp>
#include <optional>
#include <vector>

namespace bw
{
	class ClientApp;
	class ClientGamemode;
	class VirtualDirectory;

	class LocalMatch
	{
		friend class ClientSession;

		public:
			LocalMatch(ClientApp& burgApp, ClientSession& session, const Packets::MatchData& matchData);
			~LocalMatch() = default;

			inline AnimationManager& GetAnimationManager();

			void LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir);

			void Update(float elapsedTime);

		private:
			void ControlEntity(Nz::UInt32 serverId);
			Ndk::EntityHandle CreateEntity(Nz::UInt32 serverId, const std::string& entityClassName, const Nz::Vector2f& createPosition, bool hasPlayerMovement, bool isPhysical, std::optional<Nz::UInt32> parentId, Nz::UInt16 currentHealth, Nz::UInt16 maxHealth);
			void DeleteEntity(Nz::UInt32 serverId);
			void MoveEntity(Nz::UInt32 serverId, const Nz::Vector2f& newPos, const Nz::Vector2f& newLinearVel, Nz::RadianAnglef newRot, Nz::RadianAnglef newAngularVel, bool isAirControlling, bool isFacingRight);
			void PlayAnimation(Nz::UInt32 serverId, Nz::UInt8 animId);
			void SendInputs();
			void UpdateEntityHealth(Nz::UInt32 serverId, Nz::UInt16 newHealth);

			struct HealthData
			{
				float spriteWidth;
				Nz::UInt16 currentHealth;
				Nz::UInt16 maxHealth;
				Nz::SpriteRef healthSprite;
			};

			struct InputController
			{
				InputController(ClientApp& app, Nz::UInt8 playerIndex) :
				controller(app, playerIndex)
				{
				}

				LocalInputController controller;
				InputData lastInputData;
			};

			struct ServerEntity
			{
				std::optional<HealthData> health;
				Ndk::EntityOwner entity;
				Nz::RadianAnglef rotationError = 0.f;
				Nz::Vector2f positionError = Nz::Vector2f::Zero();
				bool isPhysical;
			};

			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::EventHandler, OnKeyReleased, m_onKeyReleasedSlot);

			std::optional<ClientEntityStore> m_entityStore;
			std::optional<ClientWeaponStore> m_weaponStore;
			std::shared_ptr<ClientGamemode> m_gamemode;
			std::shared_ptr<ClientScriptingContext> m_scriptingContext;
			std::string m_gamemodePath;
			std::vector<InputController> m_inputControllers;
			tsl::hopscotch_map<Nz::UInt32 /*serverEntityId*/, ServerEntity /*clientEntity*/> m_serverEntityIdToClient;
			Ndk::EntityHandle m_camera;
			Ndk::World m_world;
			AnimationManager m_animationManager;
			ClientApp& m_application;
			ClientSession& m_session;
			Packets::PlayersInput m_inputPacket;
			float m_errorCorrectionTimer;
			float m_playerEntitiesTimer;
			float m_playerInputTimer;
	};
}

#include <Client/LocalMatch.inl>

#endif