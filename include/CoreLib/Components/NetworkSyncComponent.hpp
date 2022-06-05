// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API NetworkSyncComponent
	{
		public:
			inline NetworkSyncComponent(Nz::UInt32 networkId, std::string entityClass, entt::handle parent = {});
			~NetworkSyncComponent() = default;

			inline const std::string& GetEntityClass() const;
			inline Nz::UInt32 GetNetworkId() const;
			inline entt::handle GetParent() const;

			inline void Invalidate();

			inline void UpdateParent(entt::handle parent);

			NazaraSignal(OnInvalidated, NetworkSyncComponent* /*emitter*/);

		private:
			std::string m_entityClass;
			entt::handle m_parent;
			Nz::UInt32 m_networkId;
	};
}

#include <CoreLib/Components/NetworkSyncComponent.inl>

#endif
