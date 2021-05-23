// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYER_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYER_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/Signal.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/ClientEditorLayer.hpp>
#include <ClientLib/LocalLayerEntity.hpp>
#include <ClientLib/LocalLayerSound.hpp>
#include <tsl/hopscotch_map.h>
#include <functional>
#include <memory>
#include <optional>

namespace bw
{
	class LocalMatch;

	class BURGWAR_CLIENTLIB_API LocalLayer final : public ClientEditorLayer
	{
		friend LocalMatch;

		public:
			LocalLayer(LocalMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor);
			LocalLayer(const LocalLayer&) = delete;
			LocalLayer(LocalLayer&&) noexcept;
			~LocalLayer();

			void Clear();

			inline void Disable();
			void Enable(bool enable = true);
			inline void EnablePrediction(bool enable = true);

			template<typename F> void ForEachLayerEntity(F&& func);
			template<typename F> void ForEachLayerSound(F&& func);
			void ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func) override;

			inline const Nz::Color& GetBackgroundColor() const;
			inline std::optional<std::reference_wrapper<LocalLayerEntity>> GetEntity(EntityId uniqueId);
			inline std::optional<std::reference_wrapper<LocalLayerEntity>> GetEntityByServerId(Nz::UInt32 serverId);
			inline EntityId GetUniqueIdByServerId(Nz::UInt32 serverId);
			LocalMatch& GetLocalMatch();

			bool IsEnabled() const override;
			inline bool IsPredictionEnabled() const;

			LocalLayerEntity& RegisterEntity(LocalLayerEntity layerEntity);
			LocalLayerSound& RegisterSound(LocalLayerSound layerEntity);

			void SyncVisuals();

			LocalLayer& operator=(const LocalLayer&) = delete;
			LocalLayer& operator=(LocalLayer&&) = delete;

			NazaraSignal(OnEntityCreated, LocalLayer* /*emitter*/, LocalLayerEntity& /*layerEntity*/);
			NazaraSignal(OnEntityDelete, LocalLayer* /*emitter*/, LocalLayerEntity& /*layerEntity*/);
			NazaraSignal(OnSoundCreated, LocalLayer* /*emitter*/, std::size_t /*soundIndex*/, LocalLayerSound& /*layerSound*/);
			NazaraSignal(OnSoundDelete, LocalLayer* /*emitter*/, std::size_t /*soundIndex*/, LocalLayerSound& /*layerSound*/);

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
				EntityData(LocalLayerEntity&& entity) :
				layerEntity(std::move(entity))
				{
				}

				EntityData(EntityData&& rhs) noexcept = default;

				LocalLayerEntity layerEntity;

				NazaraSlot(Ndk::Entity, OnEntityDestruction, onDestruction);
			};

			struct SoundData
			{
				SoundData(LocalLayerSound&& layerSound) :
				sound(std::move(layerSound))
				{
				}

				SoundData(SoundData&& rhs) = default;

				LocalLayerSound sound;
				std::size_t soundIndex;
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

#include <ClientLib/LocalLayer.inl>

#endif
