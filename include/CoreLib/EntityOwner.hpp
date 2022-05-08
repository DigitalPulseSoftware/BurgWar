// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ENTITYOWNER_HPP
#define BURGWAR_CORELIB_ENTITYOWNER_HPP

#include <entt/entt.hpp>

namespace bw
{
	class EntityOwner
	{
		public:
			inline EntityOwner(entt::registry& registry, entt::entity entity);
			EntityOwner(const EntityOwner&) = delete;
			inline EntityOwner(EntityOwner&& entityOwner) noexcept;
			inline ~EntityOwner();

			inline entt::entity GetEntity() const;

			operator entt::entity() const;

			EntityOwner& operator=(const EntityOwner&) = delete;
			EntityOwner& operator=(EntityOwner&&) = delete;

		private:
			entt::entity m_entity;
			entt::registry& m_registry;
	};
}

#include <CoreLib/EntityOwner.inl>

#endif
