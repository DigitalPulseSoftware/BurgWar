// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTMATCH_HPP
#define BURGWAR_CLIENTLIB_CLIENTMATCH_HPP

#include <CoreLib/AnimationManager.hpp>
#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Utility/AverageValues.hpp>
#include <ClientLib/Camera.hpp>
#include <ClientLib/Chatbox.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/EscapeMenu.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/ClientConsole.hpp>
#include <ClientLib/ClientLayer.hpp>
#include <ClientLib/ClientPlayer.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <ClientLib/Scripting/ParticleRegistry.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <Nazara/Network/UdpSocket.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/EntityOwner.hpp>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace bw
{
	class ClientEditorApp;
	class Camera;
	class ClientGamemode;
	class ClientSession;
	class InputPoller;
	class Scoreboard;
	class VirtualDirectory;

	class BURGWAR_CLIENTLIB_API ClientMatch : public SharedMatch, public std::enable_shared_from_this<ClientMatch>
	{
		friend ClientSession;

		public:
			ClientMatch(ClientEditorApp& burgApp, Nz::RenderWindow* window, Nz::RenderTarget* renderTarget, Ndk::Canvas* canvas, ClientSession& session, const Packets::AuthSuccess& authSuccess, const Packets::MatchData& matchData);
			ClientMatch(const ClientMatch&) = delete;
			ClientMatch(ClientMatch&&) = delete;
			~ClientMatch();

			template<typename T> T AdjustServerTick(T tick);

			inline EntityId AllocateClientUniqueId();

			Nz::UInt64 EstimateServerTick() const;

			void ForEachEntity(std::function<void(entt::entity entity)> func) override;
			template<typename F> void ForEachPlayer(F&& func);

			inline Nz::UInt16 GetActiveLayer();
			inline AnimationManager& GetAnimationManager();
			inline ClientAssetStore& GetAssetStore();
			inline ClientEditorApp& GetApplication();
			inline Camera& GetCamera();
			inline const Camera& GetCamera() const;
			inline ClientSession& GetClientSession();
			ClientEntityStore& GetEntityStore() override;
			const ClientEntityStore& GetEntityStore() const override;
			ClientLayer& GetLayer(LayerIndex layerIndex) override;
			const ClientLayer& GetLayer(LayerIndex layerIndex) const override;
			LayerIndex GetLayerCount() const override;
			inline ClientPlayer* GetLocalPlayerClientPlayer(std::size_t localPlayerIndex);
			inline const ClientPlayer* GetLocalPlayerClientPlayer(std::size_t localPlayerIndex) const;
			inline std::size_t GetLocalPlayerCount() const;
			inline const PlayerInputData& GetLocalPlayerInputs(std::size_t localPlayerIndex) const;
			const NetworkStringStore& GetNetworkStringStore() const override;
			inline ParticleRegistry& GetParticleRegistry();
			inline const ParticleRegistry& GetParticleRegistry() const;
			inline ClientPlayer* GetPlayerByIndex(Nz::UInt16 playerIndex);
			inline Ndk::World& GetRenderWorld();
			std::shared_ptr<const SharedGamemode> GetSharedGamemode() const override;
			ClientWeaponStore& GetWeaponStore() override;
			const ClientWeaponStore& GetWeaponStore() const override;

			void InitDebugGhosts();

			void LoadAssets(std::shared_ptr<Nz::VirtualDirectory> assetDir);
			void LoadScripts(const std::shared_ptr<Nz::VirtualDirectory>& scriptDir);

			inline void Quit();

			void RegisterEntity(EntityId uniqueId, ClientLayerEntityHandle entity);
			
			entt::entity RetrieveEntityByUniqueId(EntityId uniqueId) const override;
			EntityId RetrieveUniqueIdByEntity(entt::entity entity) const override;

			void UnregisterEntity(EntityId uniqueId);

			bool Update(float elapsedTime);

			ClientMatch& operator=(const ClientMatch&) = delete;
			ClientMatch& operator=(ClientMatch&&) = delete;

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
				Packets::EntitiesScale,
				Packets::EntityPhysics,
				Packets::EntityWeapon,
				Packets::HealthUpdate,
				Packets::MapReset,
				Packets::MatchState,
				Packets::PlayerLayer,
				Packets::PlayerWeapons
			>;

			void BindEscapeMenu();
			void BindPackets();
			void BindSignals(ClientEditorApp& burgApp, Nz::RenderWindow* window, Ndk::Canvas* canvas);
			void HandleChatMessage(const Packets::ChatMessage& packet);
			void HandleConsoleAnswer(const Packets::ConsoleAnswer& packet);
			void HandleEntityCreated(ClientLayer* layer, ClientLayerEntity& entity);
			void HandleEntityDeletion(ClientLayer* layer, ClientLayerEntity& entity);
			void HandlePlayerControlEntity(const Packets::PlayerControlEntity& packet);
			void HandlePlayerControlEntity(std::size_t playerIndex, EntityId newControlledEntityId);
			void HandlePlayerJoined(const Packets::PlayerJoined& packet);
			void HandlePlayerLeaving(const Packets::PlayerLeaving& packet);
			void HandlePlayerNameUpdate(const Packets::PlayerNameUpdate& packet);
			void HandlePlayerPingUpdate(const Packets::PlayerPingUpdate& packet);
			void HandleScriptPacket(const Packets::ScriptPacket& packet);
			void HandleTickPacket(TickPacketContent&& packet);
			void HandleTickPacket(Packets::ControlEntity&& packet);
			void HandleTickPacket(Packets::CreateEntities&& packet);
			void HandleTickPacket(Packets::DeleteEntities&& packet);
			void HandleTickPacket(Packets::DisableLayer&& packet);
			void HandleTickPacket(Packets::EnableLayer&& packet);
			void HandleTickPacket(Packets::EntitiesAnimation&& packet);
			void HandleTickPacket(Packets::EntitiesDeath&& packet);
			void HandleTickPacket(Packets::EntitiesInputs&& packet);
			void HandleTickPacket(Packets::EntitiesScale&& packet);
			void HandleTickPacket(Packets::EntityPhysics&& packet);
			void HandleTickPacket(Packets::EntityWeapon&& packet);
			void HandleTickPacket(Packets::HealthUpdate&& packet);
			void HandleTickPacket(Packets::MapReset&& packet);
			void HandleTickPacket(Packets::MatchState&& packet);
			void HandleTickPacket(Packets::PlayerLayer&& packet);
			void HandleTickPacket(Packets::PlayerWeapons&& packet);
			void HandleTickError(Nz::UInt16 serverTick, Nz::Int32 tickError);
			void InitializeRemoteConsole();
			void InitializeScoreboard();
			void OnTick(bool lastTick) override;
			void PushTickPacket(Nz::UInt16 tick, const TickPacketContent& packet);
			bool SendInputs(Nz::UInt16 serverTick, bool force);

			struct LocalPlayerData
			{
				struct Weapon
				{
					entt::entity entity;
					Nz::UInt8 category;
				};

				LocalPlayerData(Nz::UInt8 localIndex) :
				localIndex(localIndex)
				{
				}

				std::size_t selectedWeapon;
				std::shared_ptr<InputPoller> inputPoller;
				std::vector<Weapon> weapons;
				ClientLayerEntityHandle controlledEntity;
				Nz::UInt8 localIndex;
				Nz::UInt16 layerIndex = 0xFFFF;
				Nz::UInt16 playerIndex;
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

				struct WeaponData
				{
					entt::entity entity;
					bool isAttacking;
				};

				struct EntityData
				{
					Nz::RadianAnglef angularVelocity;
					Nz::RadianAnglef rotation;
					Nz::Vector2f position;
					Nz::Vector2f linearVelocity;
					bool isPhysical;
				};

				struct LayerData
				{
					LayerIndex layerIndex;
					tsl::hopscotch_map<EntityId, EntityData> entities;
				};

				struct PlayerData
				{
					PlayerInputData input;
					PlayerInputData previousInput;
					std::optional<MovementData> movement;
					std::vector<WeaponData> weapons;
				};

				Nz::UInt16 inputTick;
				std::vector<PlayerData> inputs;
				std::vector<LayerData> layers;
			};

			struct TickPrediction
			{
				Nz::UInt16 serverTick;
				Nz::Int32 tickError;
			};

			struct TickPacket
			{
				Nz::UInt16 serverTick;
				TickPacketContent content;
			};

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onRenderTargetSizeChange);
			NazaraSlot(Nz::EventHandler, OnGainedFocus, m_onGainedFocus);
			NazaraSlot(Nz::EventHandler, OnLostFocus, m_onLostFocus);
			NazaraSlot(Ndk::Canvas, OnUnhandledKeyPressed, m_onUnhandledKeyPressed);
			NazaraSlot(Ndk::Canvas, OnUnhandledKeyReleased, m_onUnhandledKeyReleased);
			NazaraSlot(ClientLayer, OnEntityCreated, m_onEntityCreated);
			NazaraSlot(ClientLayer, OnEntityDelete, m_onEntityDelete);

			typename Nz::Signal<const std::string&>::ConnectionGuard m_nicknameUpdateSlot;

			std::optional<ClientAssetStore> m_assetStore;
			std::optional<ClientEntityStore> m_entityStore;
			std::optional<ClientWeaponStore> m_weaponStore;
			std::optional<Camera> m_camera;
			std::optional<Console> m_remoteConsole;
			std::optional<Debug> m_debug;
			std::optional<ClientConsole> m_localConsole;
			std::optional<ParticleRegistry> m_particleRegistry;
			std::shared_ptr<ClientGamemode> m_gamemode;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::string m_gamemodeName;
			std::vector<std::unique_ptr<ClientLayer>> m_layers;
			std::vector<LocalPlayerData> m_localPlayers;
			std::vector<std::optional<ClientPlayer>> m_matchPlayers;
			std::vector<PredictedInput> m_predictedInputs;
			std::vector<TickPacket> m_tickedPackets;
			std::vector<TickPrediction> m_tickPredictions;
			Ndk::Canvas* m_canvas;
			Ndk::EntityHandle m_currentLayer;
			Ndk::World m_renderWorld;
			Nz::ColorBackgroundRef m_colorBackground;
			EntityId m_freeClientId;
			Nz::RenderTarget* m_renderTarget;
			Nz::RenderWindow* m_window;
			Nz::UInt16 m_activeLayerIndex;
			tsl::hopscotch_map<EntityId, ClientLayerEntityHandle> m_entitiesByUniqueId;
			tsl::hopscotch_map<EntityId, std::size_t> m_playerEntitiesByUniqueId;
			tsl::hopscotch_set<EntityId> m_inactiveEntities;
			AnimationManager m_animationManager;
			AverageValues<Nz::Int32> m_averageTickError;
			Chatbox m_chatBox;
			ClientEditorApp& m_application;
			ClientSession& m_session;
			EscapeMenu m_escapeMenu;
			PropertyValueMap m_gamemodeProperties;
			Scoreboard* m_scoreboard;
			Packets::PlayersInput m_inputPacket;
			bool m_hasFocus;
			bool m_isLeavingMatch;
			float m_errorCorrectionTimer;
			float m_playerEntitiesTimer;
			float m_playerInputTimer;
			float m_timeSinceLastInputSending;
	};
}

#include <ClientLib/ClientMatch.inl>

#endif
