// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/LogSystem/LogSink.hpp>

namespace bw
{
	void Logger::Log(const LogContext& context, std::string content) const
	{
		OverrideContent(context, content);

		LogRaw(context, content);
		if (m_logParent)
			m_logParent->LogRaw(context, content);
	}

	void Logger::LogRaw(const LogContext& context, std::string_view content) const
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

	LogContext* Logger::AllocateContext(Nz::MemoryPool& pool) const
	{
		LogContext* logContext = pool.New<LogContext>();
		InitializeContext(*logContext);

		return logContext;
	}

	void Logger::InitializeContext(LogContext& context) const
	{
		context.side = GetSide();
	}

	void Logger::OverrideContent(const LogContext& context, std::string& content) const
	{
		switch (context.side)
		{
			case LogSide::Irrelevant:
				break;

			case LogSide::Client:
				content = "[C] " + content;
				break;

			case LogSide::Editor:
				content = "[E] " + content;
				break;

			case LogSide::Server:
				content = "[S] " + content;
				break;

			default:
				break;
		}
	}

	void Logger::FreeContext(LogContext* context) const
	{
		m_contextPool.Delete(context);
	}
}
