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
#include <CoreLib/Utility/AverageValues.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <tsl/hopscotch_map.h>
#include <Nazara/Platform/EventHandler.hpp>
#include <memory>
#include <optional>
#include <variant>
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

			struct Debug
			{
				Nz::UdpSocket socket;
			};

			using TickPacketContent = std::variant<
				Packets::ControlEntity,
				Packets::CreateEntities,
				Packets::DeleteEntities,
				Packets::EntitiesAnimation,
				Packets::EntitiesInputs,
				Packets::HealthUpdate,
				Packets::MatchState
			>;

			void CreateGhostEntity(ServerEntity& serverEntity);
			void CreateHealthBar(ServerEntity& serverEntity, Nz::UInt16 currentHealth);
			void CreateName(ServerEntity& serverEntity, const std::string& name);
			const Ndk::EntityHandle& CreateReconciliationEntity(const Ndk::EntityHandle& serverEntity);
			void DebugEntityId(ServerEntity& serverEntity);
			Nz::UInt16 EstimateServerTick() const;
			void HandleTickPacket(TickPacketContent&& packet);
			void HandleTickPacket(Packets::ControlEntity&& packet);
			void HandleTickPacket(Packets::CreateEntities&& packet);
			void HandleTickPacket(Packets::DeleteEntities&& packet);
			void HandleTickPacket(Packets::EntitiesAnimation&& packet);
			void HandleTickPacket(Packets::EntitiesInputs&& packet);
			void HandleTickPacket(Packets::HealthUpdate&& packet);
			void HandleTickPacket(Packets::MatchState&& packet);
			void HandleTickError(Nz::UInt16 serverTick, Nz::Int32 tickError);
			void OnTick(bool lastTick) override;
			void PrepareClientUpdate();
			void PrepareTickUpdate();
			void ProcessInputs(float elapsedTime);
			void PushTickPacket(Nz::UInt16 tick, TickPacketContent&& packet);
			bool SendInputs(Nz::UInt16 serverTick, bool force);

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
				Ndk::EntityOwner reconciliationEntity;
				Nz::UInt8 playerIndex;
				Nz::UInt32 controlledEntityServerId;
				InputData lastInputData;
			};

			struct PredictedInput
			{
				Nz::UInt16 serverTick;
				std::vector<InputData> inputs;
			};

			struct TickPrediction
			{
				Nz::UInt16 serverTick;
				Nz::Int32 tickError;
			};

			struct PredictionData
			{
				PredictionData() : 
				reconciliationWorld(false)
				{
				}

				std::vector<PredictedInput> predictedInputs;
				Ndk::World reconciliationWorld;
				tsl::hopscotch_map<Ndk::EntityId, Ndk::EntityOwner> reconciliationEntities;
			};

			struct ServerEntity
			{
				std::optional<HealthData> health;
				std::optional<NameData> name;
				Ndk::EntityHandle serverGhost;
				Ndk::EntityOwner entity;
				Nz::RadianAnglef rotationError = 0.f;
				Nz::Vector2f positionError = Nz::Vector2f::Zero();
				Nz::UInt16 maxHealth;
				Nz::UInt32 serverEntityId;
				Nz::UInt32 weaponEntityId = 0xFFFFFFFF;
				bool isPhysical;
				bool isLocalPlayerControlled;
			};

			struct TickPacket
			{
				Nz::UInt16 tick;
				TickPacketContent content;
			};

			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::EventHandler, OnKeyReleased, m_onKeyReleasedSlot);

			std::optional<ClientEntityStore> m_entityStore;
			std::optional<ClientWeaponStore> m_weaponStore;
			std::optional<Debug> m_debug;
			std::shared_ptr<ClientGamemode> m_gamemode;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<InputController> m_inputController;
			std::string m_gamemodePath;
			std::vector<PlayerData> m_playerData;
			std::vector<TickPacket> m_tickedPackets;
			std::vector<TickPrediction> m_tickPredictions;
			tsl::hopscotch_map<Nz::UInt32 /*serverEntityId*/, ServerEntity /*clientEntity*/> m_serverEntityIdToClient;
			Ndk::EntityHandle m_camera;
			Ndk::World m_world;
			Nz::SpriteRef m_trailSpriteTest;
			Nz::UInt16 m_currentServerTick;
			AnimationManager m_animationManager;
			AverageValues<Nz::Int32> m_averageTickError;
			BurgApp& m_application;
			ClientSession& m_session;
			Packets::PlayersInput m_inputPacket;
			PredictionData m_prediction;
			float m_errorCorrectionTimer;
			float m_playerEntitiesTimer;
			float m_playerInputTimer;
			float m_timeSinceLastInputSending;
	};
}

#include <ClientLib/LocalMatch.inl>

#endif