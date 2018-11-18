// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SCRIPTEDWEAPON_HPP
#define BURGWAR_SHARED_SCRIPTING_SCRIPTEDWEAPON_HPP

#include <Shared/Scripting/ScriptedElement.hpp>
#include <Nazara/Math/Vector2.hpp>

namespace bw
{
	struct ScriptedWeapon : ScriptedElement
	{
		Nz::Vector2f spriteOrigin;
		Nz::Vector2f weaponOrigin;
		Nz::Vector2f weaponOffset;
		std::string spriteName;
		float scale;
	};
}

#endif
