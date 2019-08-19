// Copyright (C) 2019 Jérôme Leclercq
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
			SharedElementLibrary() = default;
			~SharedElementLibrary();

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterCommonLibrary(sol::table& elementMetatable);
	};
}

#include <CoreLib/Scripting/SharedElementLibrary.inl>

#endif
