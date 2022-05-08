// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/Signal.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API NetworkSyncComponent
	{
		public:
			inline NetworkSyncComponent(std::string entityClass, entt::entity parent = entt::null);
			~NetworkSyncComponent() = default;

			inline const std::string& GetEntityClass() const;
			inline entt::entity GetParent() const;

			inline void Invalidate();

			inline void UpdateParent(entt::entity parent);

			NazaraSignal(OnInvalidated, NetworkSyncComponent* /*emitter*/);

		private:
			entt::entity m_parent;
			std::string m_entityClass;
	};
}

#include <CoreLib/Components/NetworkSyncComponent.inl>

#endif
