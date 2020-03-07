// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_PLAYERCONFIGFILE_HPP
#define BURGWAR_CLIENTLIB_PLAYERCONFIGFILE_HPP

#include <CoreLib/ConfigFile.hpp>

namespace bw
{
	class BurgApp;

	class PlayerConfigFile : public ConfigFile
	{
		public:
			PlayerConfigFile(BurgApp& app);
			~PlayerConfigFile() = default;
	};
}

#include <ClientLib/PlayerConfigFile.inl>

#endif
