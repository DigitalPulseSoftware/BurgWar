// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/Logger.hpp>
#include <type_traits>

namespace bw
{
	inline Logger::Logger(BurgApp& app, LogSide logSide) :
	AbstractLogger(logSide),
	m_app(app),
	m_minimumLogLevel(LogLevel::Debug),
	m_logParent(nullptr)
	{
	}

	inline Logger::Logger(BurgApp& app, LogSide logSide, const AbstractLogger& logParent) :
	Logger(app, logSide)
	{
		m_logParent = &logParent;
	}

	template<typename... Args>
	void Logger::LogFormat(const LogContext& context, Args&&... args) const
	{
		Log(context, fmt::format(std::forward<Args>(args)...));
	}

	template<typename T>
	T* Logger::AllocateContext() const
	{
		static_assert(std::is_base_of_v<LogContext, T>);

		T* logContext = new T;
		InitializeContext(*logContext);

		return logContext;
	}

	template<typename T>
	LogContextPtr Logger::PushCustomContext() const
	{
		return LogContextPtr(this, AllocateContext<T>());
	}

	inline LogContextPtr Logger::PushContext() const
	{
		return LogContextPtr(this, NewContext());
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
