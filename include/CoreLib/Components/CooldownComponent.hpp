// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_COOLDOWNCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_COOLDOWNCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/Time.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API CooldownComponent
	{
		public:
			inline CooldownComponent(Nz::Time cooldown);
			CooldownComponent(const CooldownComponent&) = default;
			~CooldownComponent() = default;

			inline void SetNextTriggerTime(Nz::Time time);

			inline bool Trigger(Nz::Time currentTime);

		private:
			Nz::Time m_cooldown;
			Nz::Time m_nextTriggerTime;
	};
}

#include <CoreLib/Components/CooldownComponent.inl>

#endif
