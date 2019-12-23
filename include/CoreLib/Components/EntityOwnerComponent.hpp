// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_ENTITYOWNERCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_ENTITYOWNERCOMPONENT_HPP

#include <NDK/Component.hpp>
#include <NDK/EntityOwner.hpp>

namespace bw
{
	class EntityOwnerComponent : public Ndk::Component<EntityOwnerComponent>
	{
		public:
			EntityOwnerComponent() = default;
			inline EntityOwnerComponent(const EntityOwnerComponent&);
			~EntityOwnerComponent() = default;

			inline void Register(const Ndk::EntityHandle& entity);

			static Ndk::ComponentIndex componentIndex;

		private:
			std::vector<Ndk::EntityOwner> m_ownedEntities;
	};
}

#include <CoreLib/Components/EntityOwnerComponent.inl>

#endif
