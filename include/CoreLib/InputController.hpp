// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_INPUTCONTROLLER_HPP
#define BURGWAR_CORELIB_INPUTCONTROLLER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <NDK/Entity.hpp>
#include <optional>

namespace bw
{
	class BURGWAR_CORELIB_API InputController
	{
		public:
			InputController() = default;
			virtual ~InputController();

			virtual std::optional<PlayerInputData> GenerateInputs(const Ndk::EntityHandle& entity) const = 0;
	};
}

#include <CoreLib/InputController.inl>

#endif
