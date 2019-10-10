// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/Logger.hpp>
#include <type_traits>

namespace bw
{
	inline Logger::Logger() :
	m_minimumLogLevel(LogLevel::Info),
	m_logParent(nullptr)
	{
	}

	inline Logger::Logger(AbstractLogger& logParent) :
	m_minimumLogLevel(LogLevel::Info),
	m_logParent(&logParent)
	{
	}

	template<typename... Args>
	void Logger::LogFormat(LogContext& context, Args&&... args)
	{
		Log(context, fmt::format(std::forward<Args>(args)...));
	}

	inline void Logger::RegisterSink(std::shared_ptr<LogSink> sinkPtr)
	{
		m_sinks.emplace_back(std::move(sinkPtr));
	}
	
	inline void Logger::SetMinimumLogLevel(LogLevel level)
	{
		m_minimumLogLevel = level;
	}
}
