// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_ENUMS_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_ENUMS_HPP

namespace bw
{
	enum class LogLevel
	{
		Debug   = 0,
		Info    = 1,
		Warning = 2,
		Error   = 3
	};

	enum class LogSide
	{
		Irrelevant,

		Client,
		Editor,
		Server
	};
}

#endif
