// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_CLIENTWEAPONLIBRARY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_CLIENTWEAPONLIBRARY_HPP

#include <CoreLib/Scripting/SharedWeaponLibrary.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientAssetStore;

	class BURGWAR_CLIENTLIB_API ClientWeaponLibrary : public SharedWeaponLibrary
	{
		public:
			inline ClientWeaponLibrary(const Logger& logger, ClientAssetStore& assetStore);
			~ClientWeaponLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterClientLibrary(sol::table& elementMetatable);

			ClientAssetStore& m_assetStore;
	};
}

#include <ClientLib/Scripting/ClientWeaponLibrary.inl>

#endif
