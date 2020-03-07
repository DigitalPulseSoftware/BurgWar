// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SHAREDCONFIGFILE_HPP
#define BURGWAR_CORELIB_SHAREDCONFIGFILE_HPP

#include <CoreLib/ConfigFile.hpp>

namespace bw
{
	class BurgApp;

	class SharedConfigFile : public ConfigFile
	{
		public:
			SharedConfigFile(BurgApp& app);
			~SharedConfigFile() = default;
	};
}

#include <CoreLib/SharedConfigFile.inl>

#endif
