// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTWEAPONLIBRARY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTWEAPONLIBRARY_HPP

#include <CoreLib/Scripting/SharedWeaponLibrary.hpp>

namespace bw
{
	class AssetStore;

	class ClientWeaponLibrary : public SharedWeaponLibrary
	{
		public:
			inline ClientWeaponLibrary(const Logger& logger, AssetStore& assetStore);
			~ClientWeaponLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterClientLibrary(sol::table& elementMetatable);

			AssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientWeaponLibrary.inl>

#endif
