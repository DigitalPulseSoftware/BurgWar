// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_ENTITYOWNERCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_ENTITYOWNERCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <CoreLib/Components/BaseComponent.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API EntityOwnerComponent : public BaseComponent
	{
		public:
			using BaseComponent::BaseComponent;
			EntityOwnerComponent(const EntityOwnerComponent&) = delete;
			EntityOwnerComponent(EntityOwnerComponent&&) noexcept = default;
			~EntityOwnerComponent() = default;

			inline void Register(entt::handle entity);

			EntityOwnerComponent& operator=(const EntityOwnerComponent&) = delete;
			EntityOwnerComponent& operator=(EntityOwnerComponent&&) noexcept = default;

		private:
			std::vector<EntityOwner> m_ownedEntities;
	};
}

#include <CoreLib/Components/EntityOwnerComponent.inl>

#endif
