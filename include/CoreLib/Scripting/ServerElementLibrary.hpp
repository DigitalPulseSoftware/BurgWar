// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERELEMENTLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERELEMENTLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/SharedElementLibrary.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API ServerElementLibrary : public SharedElementLibrary
	{
		public:
			using SharedElementLibrary::SharedElementLibrary;
			~ServerElementLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterServerLibrary(sol::table& elementTable);

			void SetScale(entt::handle entity, float newScale) override;
	};
}

#include <CoreLib/Scripting/ServerElementLibrary.inl>

#endif
