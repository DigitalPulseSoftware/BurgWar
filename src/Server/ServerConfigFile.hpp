// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVERCONFIGFILE_HPP
#define BURGWAR_SERVERCONFIGFILE_HPP

#include <CoreLib/SharedConfigFile.hpp>

namespace bw
{
	class ServerApp;

	class ServerConfigFile : public SharedConfigFile
	{
		public:
			ServerConfigFile(ServerApp& app);
			~ServerConfigFile() = default;
	};
}

#include <Server/ServerConfigFile.inl>

#endif
