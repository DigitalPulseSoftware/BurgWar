// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_PLAYERCONFIG_HPP
#define BURGWAR_CLIENTLIB_PLAYERCONFIG_HPP

#include <CoreLib/ConfigFile.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class BurgAppComponent;

	class BURGWAR_CLIENTLIB_API PlayerConfig : public ConfigFile
	{
		public:
			PlayerConfig(BurgAppComponent& app);
			~PlayerConfig() = default;
	};
}

#include <ClientLib/PlayerConfig.inl>

#endif
