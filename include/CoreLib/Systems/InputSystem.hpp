// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_INPUTSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_INPUTSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <NDK/System.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API InputSystem : public Ndk::System<InputSystem>
	{
		public:
			InputSystem();
			~InputSystem() = default;

			static Ndk::SystemIndex systemIndex;

		private:
			void OnUpdate(float elapsedTime) override;
	};
}

#include <CoreLib/Systems/InputSystem.inl>

#endif
