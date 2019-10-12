// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERENTITYLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERENTITYLIBRARY_HPP

#include <CoreLib/Scripting/SharedEntityLibrary.hpp>

namespace bw
{
	class ServerEntityLibrary : public SharedEntityLibrary
	{
		public:
			using SharedEntityLibrary::SharedEntityLibrary;
			~ServerEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterServerLibrary(sol::table& elementMetatable);
	};
}

#include <CoreLib/Scripting/ServerEntityLibrary.inl>

#endif
