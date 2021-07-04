// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/CustomInputController.hpp>

namespace bw
{
	inline CustomInputController::CustomInputController(sol::main_protected_function callback) :
	m_callback(std::move(callback))
	{
	}
}
