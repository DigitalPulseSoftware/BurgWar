// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CUSTOMINPUTCONTROLLER_HPP
#define BURGWAR_CORELIB_CUSTOMINPUTCONTROLLER_HPP

#include <CoreLib/InputController.hpp>
#include <sol/sol.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API CustomInputController : public InputController
	{
		public:
			inline CustomInputController(sol::main_protected_function callback);
			~CustomInputController() = default;

			std::optional<PlayerInputData> GenerateInputs(const Ndk::EntityHandle& entity) const override;

		private:
			sol::main_protected_function m_callback;
	};
}

#include <CoreLib/CustomInputController.inl>

#endif
