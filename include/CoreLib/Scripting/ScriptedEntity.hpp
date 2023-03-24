// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDENTITY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDENTITY_HPP

#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <NazaraUtils/Prerequisites.hpp>

namespace bw
{
	struct ScriptedEntity : ScriptedElement
	{
		bool isNetworked;
		Nz::UInt16 maxHealth;
	};
}

#endif
