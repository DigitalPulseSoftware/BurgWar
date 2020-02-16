// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDWEAPONLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDWEAPONLIBRARY_HPP

#include <CoreLib/Scripting/AbstractElementLibrary.hpp>

namespace bw
{
	class SharedWeaponLibrary : public AbstractElementLibrary
	{
		public:
			using AbstractElementLibrary::AbstractElementLibrary;
			~SharedWeaponLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterSharedLibrary(sol::table& elementMetatable);
	};
}

#include <CoreLib/Scripting/SharedWeaponLibrary.inl>

#endif
