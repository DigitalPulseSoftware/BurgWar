// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERWEAPONLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERWEAPONLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/SharedWeaponLibrary.hpp>

namespace bw
{
	class Match;

	class BURGWAR_CORELIB_API ServerWeaponLibrary : public SharedWeaponLibrary
	{
		public:
			inline ServerWeaponLibrary(const Logger& logger, Match& match);
			~ServerWeaponLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterServerLibrary(sol::table& elementMetatable);

			Match& m_match;
	};
}

#include <CoreLib/Scripting/ServerWeaponLibrary.inl>

#endif
