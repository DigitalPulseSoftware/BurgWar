// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_BASECOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_BASECOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/Signal.hpp>
#include <entt/entt.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API BaseComponent
	{
		public:
			inline BaseComponent(entt::registry& registry, entt::entity entity);
			inline BaseComponent(const BaseComponent& component);
			inline BaseComponent(BaseComponent&& component) noexcept;
			~BaseComponent() = default;

			inline entt::entity GetEntity() const;
			inline entt::registry& GetRegistry();
			inline entt::registry& GetRegistry() const;

			void KillEntity();

			BaseComponent& operator=(const BaseComponent&) = delete;
			BaseComponent& operator=(BaseComponent&&) = delete;

		private:
			entt::entity m_entity;
			entt::registry& m_registry;
	};
}

#include <CoreLib/Components/BaseComponent.inl>

#endif
