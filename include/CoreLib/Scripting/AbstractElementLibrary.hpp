// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ABSTRACTELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_ABSTRACTELEMENTLIBRARY_HPP

#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <NDK/Entity.hpp>
#include <Thirdparty/sol3/forward.hpp>
#include <memory>

namespace bw
{
	class Logger;

	class AbstractElementLibrary
	{
		public:
			inline AbstractElementLibrary(const Logger& logger);
			virtual ~AbstractElementLibrary();

			inline const Logger& GetLogger() const;

			virtual void RegisterLibrary(sol::table& elementMetatable) = 0;

			static std::shared_ptr<ScriptedElement> AssertScriptElement(const sol::table& entityTable);
			static Ndk::EntityHandle AssertScriptEntity(const sol::table& entityTable);
			static std::shared_ptr<ScriptedElement> RetrieveScriptElement(const sol::table& entityTable);
			static Ndk::EntityHandle RetrieveScriptEntity(const sol::table& entityTable);

		private:
			const Logger& m_logger;
	};
}

#include <CoreLib/Scripting/AbstractElementLibrary.inl>

#endif
