// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_NETWORKSYNCCOMPONENT_HPP
#define BURGWAR_SHARED_COMPONENTS_NETWORKSYNCCOMPONENT_HPP

#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class NetworkSyncComponent : public Ndk::Component<NetworkSyncComponent>
	{
		public:
			inline NetworkSyncComponent(std::string entityClass);
			~NetworkSyncComponent() = default;

			inline const std::string& GetEntityClass() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			std::string m_entityClass;
	};
}

#include <Shared/Components/NetworkSyncComponent.inl>

#endif
