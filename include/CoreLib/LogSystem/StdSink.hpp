// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_STDSINK_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_STDSINK_HPP

#include <CoreLib/LogSystem/LogContext.hpp>
#include <CoreLib/LogSystem/LogSink.hpp>

namespace bw
{
	class StdSink : public LogSink
	{
		public:
			StdSink() = default;
			~StdSink() = default;

			void Write(const LogContext& context, std::string_view content) override;
	};
}

#include <CoreLib/LogSystem/StdSink.inl>

#endif
