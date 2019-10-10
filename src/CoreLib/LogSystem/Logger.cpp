// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/LogSystem/LogSink.hpp>

namespace bw
{
	void Logger::Log(LogContext& context, std::string content)
	{
		OverrideContent(context, content);

		LogRaw(context, content);
		if (m_logParent)
			m_logParent->LogRaw(context, content);
	}

	void Logger::LogRaw(LogContext& context, std::string_view content)
	{
		for (auto& sinkPtr : m_sinks)
			sinkPtr->Write(context, content);
	}

	bool Logger::ShouldLog(const LogContext& context) const
	{
		if (context.level < m_minimumLogLevel)
			return false;

		if (m_logParent && !m_logParent->ShouldLog(context))
			return false;

		return true;
	}

	void Logger::OverrideContent(const LogContext& /*context*/, std::string& /*content*/)
	{
	}
}
