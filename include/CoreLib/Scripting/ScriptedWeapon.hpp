// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDWEAPON_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDWEAPON_HPP

#include <CoreLib/AnimationStore.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <memory>

namespace bw
{
	struct ScriptedWeapon : ScriptedElement
	{
		WeaponAttackMode attackMode;
		Nz::UInt8 category;
		Nz::UInt32 cooldown;
		Nz::Vector2f spriteOrigin;
		Nz::Vector2f weaponOrigin;
		std::shared_ptr<AnimationStore> animations;
		std::string spriteName;
		float scale;
		sol::main_protected_function animationStartFunction;
	};
}

#endif
