// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTELEMENTLIBRARY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTELEMENTLIBRARY_HPP

#include <CoreLib/Scripting/SharedElementLibrary.hpp>

namespace bw
{
	class ClientElementLibrary : public SharedElementLibrary
	{
		public:
			using SharedElementLibrary::SharedElementLibrary;
			~ClientElementLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterClientLibrary(sol::table& elementTable);
	};
}

#include <ClientLib/Scripting/ClientElementLibrary.inl>

#endif
