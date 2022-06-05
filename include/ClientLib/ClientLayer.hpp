// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTLAYER_HPP
#define BURGWAR_CLIENTLIB_CLIENTLAYER_HPP

#include <Nazara/Utils/Bitset.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/ClientEditorLayer.hpp>
#include <ClientLib/ClientLayerEntity.hpp>
#include <ClientLib/ClientLayerSound.hpp>
#include <tsl/hopscotch_map.h>
#include <functional>
#include <memory>
#include <optional>

namespace bw
{
	class ClientMatch;

	class BURGWAR_CLIENTLIB_API ClientLayer final : public ClientEditorLayer
	{
		friend ClientMatch;

		public:
			ClientLayer(ClientMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor);
			ClientLayer(const ClientLayer&) = delete;
			ClientLayer(ClientLayer&&) noexcept;
			~ClientLayer();

			inline void Disable();
			void Enable(bool enable = true);
			inline void EnablePrediction(bool enable = true);

			template<typename F> void ForEachLayerEntity(F&& func);
			template<typename F> void ForEachLayerSound(F&& func);
			void ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func) override;

			inline const Nz::Color& GetBackgroundColor() const;
			inline std::optional<std::reference_wrapper<ClientLayerEntity>> GetEntity(EntityId uniqueId);
			inline std::optional<std::reference_wrapper<ClientLayerEntity>> GetEntityByServerId(Nz::UInt32 serverId);
			inline EntityId GetUniqueIdByServerId(Nz::UInt32 serverId);
			ClientMatch& GetClientMatch();

			bool IsEnabled() const override;
			inline bool IsPredictionEnabled() const;

			void PostFrameUpdate(float elapsedTime) override;

			ClientLayerEntity& RegisterEntity(ClientLayerEntity layerEntity);
			ClientLayerSound& RegisterSound(ClientLayerSound layerEntity);

			void SyncVisuals();

			ClientLayer& operator=(const ClientLayer&) = delete;
			ClientLayer& operator=(ClientLayer&&) = delete;

			NazaraSignal(OnEntityCreated, ClientLayer* /*emitter*/, ClientLayerEntity& /*layerEntity*/);
			NazaraSignal(OnEntityDelete, ClientLayer* /*emitter*/, ClientLayerEntity& /*layerEntity*/);
			NazaraSignal(OnSoundCreated, ClientLayer* /*emitter*/, std::size_t /*soundIndex*/, ClientLayerSound& /*layerSound*/);
			NazaraSignal(OnSoundDelete, ClientLayer* /*emitter*/, std::size_t /*soundIndex*/, ClientLayerSound& /*layerSound*/);

		private:
			void CreateEntity(Nz::UInt32 entityId, const Packets::Helper::EntityData& entityData);
			void HandleEntityDestruction(EntityId uniqueId);
			void HandlePacket(const Packets::CreateEntities::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::DeleteEntities::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EnableLayer::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesAnimation::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesDeath::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesInputs::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesScale::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntityPhysics& packet);
			void HandlePacket(const Packets::EntityWeapon& packet);
			void HandlePacket(const Packets::HealthUpdate::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::MapReset::Entity* entities, std::size_t entityCount);

			struct EntityData
			{
				EntityData(ClientLayerEntity&& entity) :
				layerEntity(std::move(entity))
				{
				}

				EntityData(EntityData&& rhs) noexcept = default;

				ClientLayerEntity layerEntity;

				NazaraSlot(Ndk::Entity, OnEntityDestruction, onDestruction);
			};

			struct SoundData
			{
				SoundData(ClientLayerSound&& layerSound) :
				sound(std::move(layerSound))
				{
				}

				SoundData(SoundData&& rhs) = default;

				ClientLayerSound sound;
			};

			tsl::hopscotch_map<EntityId /*uniqueId*/, EntityData> m_entities;
			tsl::hopscotch_map<Nz::UInt32 /*serverEntityId*/, EntityId> m_serverEntityIds;
			std::vector<std::optional<SoundData>> m_sounds;
			Nz::Bitset<Nz::UInt64> m_freeSoundIds;
			Nz::Color m_backgroundColor;
			bool m_isEnabled;
			bool m_isPredictionEnabled;
	};
}

#include <ClientLib/ClientLayer.inl>

#endif
