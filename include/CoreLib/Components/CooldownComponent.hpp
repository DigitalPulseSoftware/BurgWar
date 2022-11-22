// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_COOLDOWNCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_COOLDOWNCOMPONENT_HPP

#include <CoreLib/Export.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API CooldownComponent
	{
		public:
			inline CooldownComponent(Nz::UInt32 cooldown);
			CooldownComponent(const CooldownComponent&) = default;
			~CooldownComponent() = default;

			inline void SetNextTriggerTime(Nz::UInt64 time);

			inline bool Trigger(Nz::UInt64 currentTime);

		private:
			Nz::UInt32 m_cooldown;
			Nz::UInt64 m_nextTriggerTime;
	};
}

#include <CoreLib/Components/CooldownComponent.inl>

#endif
