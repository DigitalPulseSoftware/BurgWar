// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ELEMENTEVENTCONNECTION_HPP
#define BURGWAR_CORELIB_SCRIPTING_ELEMENTEVENTCONNECTION_HPP

#include <CoreLib/Scripting/ElementEvents.hpp>
#include <cstddef>
#include <variant>

namespace bw
{
	struct ElementEventConnection
	{
		std::variant<ElementEvent, std::size_t> event;
		std::size_t callbackId;
	};
}

#endif
