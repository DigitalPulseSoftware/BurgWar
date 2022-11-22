// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_PLAYERINPUTCONTROLLER_HPP
#define BURGWAR_CORELIB_PLAYERINPUTCONTROLLER_HPP

#include <CoreLib/InputController.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API PlayerInputController : public InputController
	{
		public:
			PlayerInputController() = default;
			~PlayerInputController() = default;

			std::optional<PlayerInputData> GenerateInputs(entt::handle entity) const override;
	};
}

#include <CoreLib/PlayerInputController.inl>

#endif
