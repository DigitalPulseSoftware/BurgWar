// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ABSTRACTELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_ABSTRACTELEMENTLIBRARY_HPP

#include <NDK/Entity.hpp>
#include <sol3/forward.hpp>
#include <memory>

namespace bw
{
	class AbstractElementLibrary
	{
		public:
			AbstractElementLibrary() = default;
			virtual ~AbstractElementLibrary();

			virtual void RegisterLibrary(sol::table& elementMetatable) = 0;

			static const Ndk::EntityHandle& AssertScriptEntity(const sol::table& entityTable);
	};
}

#include <CoreLib/Scripting/AbstractElementLibrary.inl>

#endif
