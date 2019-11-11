// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYER_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYER_HPP

#include <Nazara/Core/Signal.hpp>
#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <ClientLib/LocalLayerEntity.hpp>
#include <tsl/hopscotch_map.h>
#include <functional>
#include <memory>
#include <optional>

namespace bw
{
	class LocalMatch;

	class LocalLayer : public SharedLayer
	{
		friend LocalMatch;

		public:
			LocalLayer(LocalMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor);
			LocalLayer(const LocalLayer&) = delete;
			LocalLayer(LocalLayer&&) noexcept = default;
			~LocalLayer() = default;

			inline void Disable();
			void Enable(bool enable = true);

			template<typename F> void ForEachLayerEntity(F&& func);

			inline const Nz::Color& GetBackgroundColor() const;
			inline std::optional<std::reference_wrapper<LocalLayerEntity>> GetEntity(Nz::UInt32 serverId);
			LocalMatch& GetLocalMatch();

			inline bool IsEnabled() const;

			void SyncVisuals();

			LocalLayer& operator=(const LocalLayer&) = delete;
			LocalLayer& operator=(LocalLayer&&) noexcept = default;

			NazaraSignal(OnDisabled, LocalLayer* /*emitter*/);
			NazaraSignal(OnEnabled, LocalLayer* /*emitter*/);
			NazaraSignal(OnEntityCreated, LocalLayer* /*emitter*/, LocalLayerEntity& /*layerEntity*/);
			NazaraSignal(OnEntityDelete, LocalLayer* /*emitter*/, LocalLayerEntity& /*layerEntity*/);

		private:
			void CreateEntity(Nz::UInt32 entityId, const Packets::Helper::EntityData& entityData);
			void HandlePacket(const Packets::CreateEntities::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::DeleteEntities::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EnableLayer::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesAnimation::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesDeath::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::EntitiesInputs::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::HealthUpdate::Entity* entities, std::size_t entityCount);
			void HandlePacket(const Packets::MatchState::Entity* entities, std::size_t entityCount);

			tsl::hopscotch_map<Nz::UInt32 /*serverEntityId*/, LocalLayerEntity /*localEntity*/> m_serverEntityIdToClient;
			Nz::Color m_backgroundColor;
			bool m_isEnabled;
	};
}

#include <ClientLib/LocalLayer.inl>

#endif