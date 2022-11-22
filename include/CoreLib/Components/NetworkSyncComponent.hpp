// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Utils/MovablePtr.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class NetworkSyncSystem;

	class BURGWAR_CORELIB_API NetworkSyncComponent
	{
		public:
			NetworkSyncComponent(NetworkSyncSystem& networkSync, std::string entityClass, entt::handle parent = {});
			NetworkSyncComponent(const NetworkSyncComponent&) = delete;
			inline NetworkSyncComponent(NetworkSyncComponent&& networkComponent) noexcept;
			~NetworkSyncComponent();

			inline const std::string& GetEntityClass() const;
			inline Nz::UInt32 GetNetworkId() const;
			inline entt::handle GetParent() const;

			inline void Invalidate();

			inline void UpdateParent(entt::handle parent);

			NetworkSyncComponent& operator=(const NetworkSyncComponent&) = delete;
			inline NetworkSyncComponent& operator=(NetworkSyncComponent&& networkComponent) noexcept;

			NazaraSignal(OnInvalidated, NetworkSyncComponent* /*emitter*/);

		private:
			static constexpr Nz::UInt32 InvalidNetworkId = 0xFFFFFFFF;

			std::string m_entityClass;
			entt::handle m_parent;
			Nz::MovablePtr<NetworkSyncSystem> m_networkSystem;
			Nz::UInt32 m_networkId;
	};
}

#include <CoreLib/Components/NetworkSyncComponent.inl>

#endif
