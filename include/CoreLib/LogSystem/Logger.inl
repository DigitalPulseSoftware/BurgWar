// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/Logger.hpp>
#include <type_traits>

namespace bw
{
	template<typename T, typename Context>
	Logger<T, Context>::Logger() :
	m_minimumLogLevel(LogLevel::Info),
	m_logParent(nullptr)
	{
	}

	template<typename T, typename Context>
	Logger<T, Context>::Logger(T& logParent) :
	m_minimumLogLevel(LogLevel::Info),
	m_logParent(&logParent)
	{
	}

	template<typename T, typename Context>
	template<typename... Args>
	void Logger<T, Context>::Log(Context& context, Args&& ... args)
	{
		if (!ShouldLog(context))
			return;

		std::string content = fmt::format(std::forward<Args>(args)...);
		OverrideContent(context, content);

		LogRaw(context, content);

		if constexpr (!std::is_same_v<T, NoLogger>)
		{
			if (m_logParent)
				m_logParent->LogRaw(context, content);
		}
	}

	template<typename T, typename Context>
	void Logger<T, Context>::LogRaw(Context& context, std::string_view content)
	{
		for (auto& sinkPtr : m_sinks)
			sinkPtr->Write(context, content);
	}

	template<typename T, typename Context>
	void Logger<T, Context>::RegisterSink(std::shared_ptr<LogSink<Context>> sinkPtr)
	{
		m_sinks.emplace_back(std::move(sinkPtr));
	}

	template<typename T, typename Context>
	void Logger<T, Context>::SetMinimumLogLevel(LogLevel level)
	{
		m_minimumLogLevel = level;
	}

	template<typename T, typename Context>
	bool Logger<T, Context>::ShouldLog(const Context& context) const
	{
		if (context.level < m_minimumLogLevel)
			return false;

		if constexpr (!std::is_same_v<T, NoLogger>)
		{
			if (m_logParent)
				return m_logParent->ShouldLog(context);
		}
		
		return true;
	}

	template<typename T, typename Context>
	void Logger<T, Context>::OverrideContent(const Context& /*context*/, std::string& /*content*/)
	{
	}
}
