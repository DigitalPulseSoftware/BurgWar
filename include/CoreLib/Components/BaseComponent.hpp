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
			inline BaseComponent(entt::handle handle);
			BaseComponent(const BaseComponent&) = default;
			BaseComponent(BaseComponent&&) noexcept = default;
			~BaseComponent() = default;

			inline entt::entity GetEntity() const;
			inline entt::handle GetHandle() const;
			inline entt::registry* GetRegistry();
			inline entt::registry* GetRegistry() const;

			void KillEntity();

			BaseComponent& operator=(const BaseComponent&) = default;
			BaseComponent& operator=(BaseComponent&&) = default;

		private:
			entt::handle m_handle;
	};
}

#include <CoreLib/Components/BaseComponent.inl>

#endif
