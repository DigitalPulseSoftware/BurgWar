// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDWEAPONSTORE_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/ScriptedWeapon.hpp>
#include <CoreLib/Scripting/ScriptStore.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API SharedWeaponStore : public ScriptStore<ScriptedWeapon>
	{
		public:
			SharedWeaponStore(const Logger& logger, std::shared_ptr<ScriptingContext> context, bool isServer);
			~SharedWeaponStore() = default;

		protected:
			void InitializeElement(sol::main_table& elementTable, ScriptedWeapon& weapon) override = 0;
			bool InitializeWeapon(const ScriptedWeapon& weaponClass, entt::entity entity, entt::entity parent);
	};
}

#include <CoreLib/Scripting/SharedWeaponStore.inl>

#endif
