// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_INPUTSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_INPUTSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API InputSystem
	{
		public:
			InputSystem();
			~InputSystem() = default;

		private:
			void OnUpdate(float elapsedTime);
	};
}

#include <CoreLib/Systems/InputSystem.inl>

#endif
