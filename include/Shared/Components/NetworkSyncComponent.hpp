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
			inline NetworkSyncComponent(std::string entityClass, const Ndk::EntityHandle& parent = Ndk::EntityHandle::InvalidHandle);
			~NetworkSyncComponent() = default;

			inline const std::string& GetEntityClass() const;
			inline const Ndk::EntityHandle& GetParent() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Ndk::EntityHandle m_parent;
			std::string m_entityClass;
	};
}

#include <Shared/Components/NetworkSyncComponent.inl>

#endif
