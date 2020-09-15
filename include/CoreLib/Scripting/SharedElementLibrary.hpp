// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDELEMENTLIBRARY_HPP

#include <CoreLib/Scripting/AbstractElementLibrary.hpp>

namespace bw
{
	class SharedElementLibrary : public AbstractElementLibrary
	{
		public:
			using AbstractElementLibrary::AbstractElementLibrary;
			~SharedElementLibrary();

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterCommonLibrary(sol::table& elementMetatable);
			void RegisterCustomEvent(const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async);
			void RegisterEvent(const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async);
	};
}

#include <CoreLib/Scripting/SharedElementLibrary.inl>

#endif
