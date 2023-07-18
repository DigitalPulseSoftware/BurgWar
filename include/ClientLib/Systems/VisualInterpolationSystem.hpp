// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SYSTEMS_VISUALINTERPOLATIONSYSTEM_HPP
#define BURGWAR_CLIENTLIB_SYSTEMS_VISUALINTERPOLATIONSYSTEM_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Core/Time.hpp>
#include <entt/entt.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API VisualInterpolationSystem
	{
		public:
			VisualInterpolationSystem(entt::registry& registry);
			~VisualInterpolationSystem() = default;

			void Update(Nz::Time elapsedTime);

		private:
			entt::observer m_observer;
			entt::registry& m_registry;
	};
}

#include <ClientLib/Systems/VisualInterpolationSystem.inl>

#endif
