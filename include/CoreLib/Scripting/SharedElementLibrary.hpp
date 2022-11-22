// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDELEMENTLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <entt/entt.hpp>

namespace bw
{
	struct ElementEventConnection;

	class BURGWAR_CORELIB_API SharedElementLibrary : public AbstractElementLibrary
	{
		public:
			using AbstractElementLibrary::AbstractElementLibrary;
			~SharedElementLibrary();

			void RegisterLibrary(sol::table& elementMetatable) override;

		protected:
			virtual void SetScale(entt::handle entity, float newScale) = 0;

		private:
			void RegisterCommonLibrary(sol::table& elementMetatable);
			ElementEventConnection RegisterCustomEvent(const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async);
			ElementEventConnection RegisterEvent(lua_State* L, const sol::table& entityTable, const std::string_view& event, sol::main_protected_function callback, bool async);
	};
}

#include <CoreLib/Scripting/SharedElementLibrary.inl>

#endif
