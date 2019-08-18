// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SHAREDELEMENTLIBRARY_HPP

#include <NDK/Entity.hpp>
#include <sol3/forward.hpp>

namespace bw
{
	class SharedMatch;

	class SharedElementLibrary
	{
		public:
			SharedElementLibrary() = default;
			virtual ~SharedElementLibrary();

			virtual void RegisterLibrary(SharedMatch& match) = 0;

			static const Ndk::EntityHandle& AssertScriptEntity(const sol::table& entityTable);

		protected:
			void RegisterCommonLibrary(sol::table& elementTable);
	};
}

#include <CoreLib/Scripting/SharedElementLibrary.inl>

#endif
