// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTCONFIGFILE_HPP
#define BURGWAR_CLIENTCONFIGFILE_HPP

#include <CoreLib/SharedConfigFile.hpp>

namespace bw
{
	class ClientApp;

	class ClientConfigFile : public SharedConfigFile
	{
		public:
			ClientConfigFile(ClientApp& app);
			~ClientConfigFile() = default;
	};
}

#include <Client/ClientConfigFile.inl>

#endif
