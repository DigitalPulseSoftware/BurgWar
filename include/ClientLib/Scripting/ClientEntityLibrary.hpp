// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTENTITYLIBRARY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTENTITYLIBRARY_HPP

#include <CoreLib/Scripting/SharedEntityLibrary.hpp>

namespace bw
{
	class AssetStore;
	class Logger;

	class ClientEntityLibrary : public SharedEntityLibrary
	{
		public:
			inline ClientEntityLibrary(const Logger& logger, AssetStore& assetStore);
			~ClientEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterClientLibrary(sol::table& elementMetatable);

			AssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientEntityLibrary.inl>

#endif
