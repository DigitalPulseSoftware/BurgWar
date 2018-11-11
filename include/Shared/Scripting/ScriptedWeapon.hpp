// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SCRIPTEDWEAPON_HPP
#define BURGWAR_SHARED_SCRIPTING_SCRIPTEDWEAPON_HPP

#include <Shared/Scripting/ScriptedElement.hpp>

namespace bw
{
	struct ScriptedWeapon : ScriptedElement
	{
		std::string spriteName;
		float scale;
	};
}

#endif
