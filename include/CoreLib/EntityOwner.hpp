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
			inline EntityOwner(entt::handle entity = entt::handle{});
			EntityOwner(const EntityOwner&) = delete;
			inline EntityOwner(EntityOwner&& entityOwner) noexcept;
			inline ~EntityOwner();

			inline entt::handle GetEntity() const;

			inline explicit operator bool() const;
			inline operator entt::handle() const;
			inline entt::handle* operator->();
			inline const entt::handle* operator->() const;

			EntityOwner& operator=(const EntityOwner&) = delete;
			inline EntityOwner& operator=(EntityOwner&& entityOwner) noexcept;

		private:
			entt::handle m_entity;
	};
}

#include <CoreLib/EntityOwner.inl>

#endif
