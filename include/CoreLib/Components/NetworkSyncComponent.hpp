// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_NETWORKSYNCCOMPONENT_HPP

#include <Nazara/Core/Signal.hpp>
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

			inline void Invalidate();

			static Ndk::ComponentIndex componentIndex;

			NazaraSignal(OnInvalidated, NetworkSyncComponent* /*emitter*/);

		private:
			Ndk::EntityHandle m_parent;
			std::string m_entityClass;
	};
}

#include <CoreLib/Components/NetworkSyncComponent.inl>

#endif
