// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGSINK_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGSINK_HPP

#include <CoreLib/Export.hpp>
#include <string_view>

namespace bw
{
	struct LogContext;

	class BURGWAR_CORELIB_API LogSink
	{
		public:
			LogSink() = default;
			virtual ~LogSink() = default;

			virtual void Write(const LogContext& context, std::string_view content) = 0;
	};
}

#endif
