// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDENTITY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDENTITY_HPP

#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <Nazara/Prerequisites.hpp>

namespace bw
{
	struct ScriptedEntity : ScriptedElement
	{
		bool isNetworked;
		sol::protected_function initializeFunction;
		Nz::UInt16 maxHealth;
	};
}

#endif
