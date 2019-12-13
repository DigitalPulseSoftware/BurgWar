// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALMATCH_HPP
#define BURGWAR_CLIENTLIB_LOCALMATCH_HPP

#include <CoreLib/AnimationManager.hpp>
#include <CoreLib/AssetStore.hpp>
#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Utility/AverageValues.hpp>
#include <ClientLib/Chatbox.hpp>
#include <ClientLib/LocalConsole.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/LocalMatchPrediction.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/EntityOwner.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace bw
{
	class BurgApp;
	class ClientGamemode;
	class ClientSession;
	class InputController;
	class VirtualDirectory;

	class LocalMatch : public SharedMatch, public std::enable_shared_from_this<LocalMatch>
	{
		friend ClientSession;

		public:
			LocalMatch(BurgApp& burgApp, Nz::RenderWindow* window, Ndk::Canvas* canvas, ClientSession& session, const Packets::MatchData& matchData);
			LocalMatch(const LocalMatch&) = delete;
			LocalMatch(LocalMatch&&) = delete;
			~LocalMatch();

			template<typename T> T AdjustServerTick(T tick);
			Nz::UInt64 EstimateServerTick() const;

			void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) override;

			inline Nz::UInt16 GetActiveLayer();
			inline AnimationManager& GetAnimationManager();
			inline AssetStore& GetAssetStore();
			inline BurgApp& GetApplication();
			inline const Ndk::EntityHandle& GetCameraEntity() const;
			inline ClientSession& GetClientSession();
			ClientEntityStore& GetEntityStore() override;
			const ClientEntityStore& GetEntityStore() const override;
			LocalLayer& GetLayer(LayerIndex layerIndex) override;
			const LocalLayer& GetLayer(LayerIndex layerIndex) const override;
			LayerIndex GetLayerCount() const override;
			inline Ndk::World& GetRenderWorld();
			ClientWeaponStore& GetWeaponStore() override;
			const ClientWeaponStore& GetWeaponStore() const override;

			void InitDebugGhosts();

			void LoadAssets(std::shared_ptr<VirtualDirectory> assetDir);
			void LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir);

			void Update(float elapsedTime);

			LocalMatch& operator=(const LocalMatch&) = delete;
			LocalMatch& operator=(LocalMatch&&) = delete;

			NazaraSignal(OnCameraMoved, LocalMatch* /*match*/, const Nz::Vector2f& /*newPosition*/);

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
				Packets::DisableLayer,
				Packets::EnableLayer,
				Packets::EntitiesAnimation,
				Packets::EntitiesDeath,
				Packets::EntitiesInputs,
				Packets::EntityWeapon,
				Packets::HealthUpdate,
				Packets::MatchState,
				Packets::PlayerLayer,
				Packets::PlayerWeapons
			>;

			void BindPackets();
			void HandleChatMessage(const Packets::ChatMessage& packet);
			void HandleConsoleAnswer(const Packets::ConsoleAnswer& packet);
			void HandleTickPacket(TickPacketContent&& packet);
			void HandleTickPacket(Packets::ControlEntity&& packet);
			void HandleTickPacket(Packets::CreateEntities&& packet);
			void HandleTickPacket(Packets::DeleteEntities&& packet);
			void HandleTickPacket(Packets::DisableLayer&& packet);
			void HandleTickPacket(Packets::EnableLayer&& packet);
			void HandleTickPacket(Packets::EntitiesAnimation&& packet);
			void HandleTickPacket(Packets::EntitiesDeath&& packet);
			void HandleTickPacket(Packets::EntitiesInputs&& packet);
			void HandleTickPacket(Packets::EntityWeapon&& packet);
			void HandleTickPacket(Packets::HealthUpdate&& packet);
			void HandleTickPacket(Packets::MatchState&& packet);
			void HandleTickPacket(Packets::PlayerLayer&& packet);
			void HandleTickPacket(Packets::PlayerWeapons&& packet);
			void HandleTickError(Nz::UInt16 serverTick, Nz::Int32 tickError);
			void InitializeRemoteConsole();
			void OnTick(bool lastTick) override;
			void ProcessInputs(float elapsedTime);
			void PushTickPacket(Nz::UInt16 tick, const TickPacketContent& packet);
			bool SendInputs(Nz::UInt16 serverTick, bool force);

			struct PlayerData
			{
				PlayerData(Nz::UInt8 playerIndex) :
				playerIndex(playerIndex)
				{
				}

				std::size_t selectedWeapon;
				std::shared_ptr<InputController> inputController;
				std::vector<Ndk::EntityHandle> weapons;
				LocalLayerEntityHandle controlledEntity;
				Nz::UInt8 playerIndex;
				Nz::UInt16 layerIndex = 0xFFFF;
				PlayerInputData lastInputData;
			};

			struct PredictedInput
			{
				struct MovementData
				{
					Nz::Vector2f surfaceVelocity;
					bool wasJumping;
					bool isOnGround;
					float jumpTime;
					float friction;
				};

				struct PlayerData
				{
					PlayerInputData input;
					std::optional<MovementData> movement;
				};

				Nz::UInt16 serverTick;
				std::vector<PlayerData> inputs;
			};

			struct TickPrediction
			{
				Nz::UInt16 serverTick;
				Nz::Int32 tickError;
			};

			struct TickPacket
			{
				Nz::UInt16 tick;
				TickPacketContent content;
			};

			NazaraSlot(Nz::EventHandler, OnGainedFocus, m_onGainedFocus);
			NazaraSlot(Nz::EventHandler, OnLostFocus, m_onLostFocus);
			NazaraSlot(Ndk::Canvas, OnUnhandledKeyPressed, m_onUnhandledKeyPressed);
			NazaraSlot(LocalLayer, OnEntityCreated, m_onEntityCreated);
			NazaraSlot(LocalLayer, OnEntityDelete, m_onEntityDelete);

			std::optional<AssetStore> m_assetStore;
			std::optional<ClientEntityStore> m_entityStore;
			std::optional<ClientWeaponStore> m_weaponStore;
			std::optional<Console> m_remoteConsole;
			std::optional<Debug> m_debug;
			std::optional<LocalConsole> m_localConsole;
			std::optional<LocalMatchPrediction> m_prediction;
			std::shared_ptr<ClientGamemode> m_gamemode;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::string m_gamemodePath;
			std::vector<std::unique_ptr<LocalLayer>> m_layers;
			std::vector<PlayerData> m_playerData;
			std::vector<PredictedInput> m_predictedInputs;
			std::vector<TickPacket> m_tickedPackets;
			std::vector<TickPrediction> m_tickPredictions;
			Ndk::Canvas* m_canvas;
			Ndk::EntityHandle m_camera;
			Ndk::EntityHandle m_currentLayer;
			Ndk::World m_renderWorld;
			Nz::ColorBackgroundRef m_colorBackground;
			Nz::RenderWindow* m_window;
			Nz::UInt16 m_activeLayerIndex;
			AnimationManager m_animationManager;
			AverageValues<Nz::Int32> m_averageTickError;
			BurgApp& m_application;
			Chatbox m_chatBox;
			ClientSession& m_session;
			Packets::PlayersInput m_inputPacket;
			bool m_hasFocus;
			float m_errorCorrectionTimer;
			float m_playerEntitiesTimer;
			float m_playerInputTimer;
			float m_timeSinceLastInputSending;
	};
}

#include <ClientLib/LocalMatch.inl>

#endif