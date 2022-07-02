// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_INPUTSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_INPUTSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API InputSystem
	{
		public:
			static constexpr Nz::Int64 ExecutionOrder = -1000;

			inline InputSystem(entt::registry& registry);
			~InputSystem() = default;

			void Update(float elapsedTime);

		private:
			entt::registry& m_registry;
	};
}

#include <CoreLib/Systems/InputSystem.inl>

#endif
