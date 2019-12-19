// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/Logger.hpp>
#include <type_traits>

namespace bw
{
	inline Logger::Logger(LogSide logSide, std::size_t contextSize) :
	AbstractLogger(logSide),
	m_contextPool(static_cast<unsigned int>(contextSize), 4),
	m_minimumLogLevel(LogLevel::Debug),
	m_logParent(nullptr)
	{
	}

	inline Logger::Logger(LogSide logSide, const AbstractLogger& logParent, std::size_t contextSize) :
	Logger(logSide, contextSize)
	{
		m_logParent = &logParent;
	}

	inline Logger::Logger(const Logger& logger) :
	AbstractLogger(logger),
	m_contextPool(logger.m_contextPool.GetBlockSize()),
	m_minimumLogLevel(logger.m_minimumLogLevel),
	m_logParent(logger.m_logParent),
	m_sinks(logger.m_sinks)
	{
	}

	template<typename... Args>
	void Logger::LogFormat(const LogContext& context, Args&&... args) const
	{
		Log(context, fmt::format(std::forward<Args>(args)...));
	}

	inline LogContextPtr Logger::PushContext() const
	{
		LogContextPtr context(this, AllocateContext(m_contextPool));
		InitializeContext(*context);

		return context;
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
