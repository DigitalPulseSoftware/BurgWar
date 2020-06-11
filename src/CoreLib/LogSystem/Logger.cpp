// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/LogSystem/LogSink.hpp>
#include <array>
#include <charconv>
#include <sstream>

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

	LogContext* Logger::NewContext(Nz::MemoryPool& pool) const
	{
		return AllocateContext<LogContext>(pool);
	}

	void Logger::InitializeContext(LogContext& context) const
	{
		context.elapsedTime = m_app.GetAppTime() / 1000.f;
		context.side = GetSide();
	}

	void Logger::OverrideContent(const LogContext& context, std::string& content) const
	{
#ifdef _MSC_VER
		auto TimePart = [](float elapsedTime) -> std::string
		{
			std::array<char, 40> buffer;
			buffer[0] = '[';

			auto result = std::to_chars(buffer.data() + 1, buffer.data() + buffer.size() - 2, elapsedTime, std::chars_format::fixed, 3);
			assert(result.ec == std::errc{});

			*result.ptr = ']';

			return std::string(buffer.data(), result.ptr - buffer.data() + 1);
		};
#else
		// libstdc++ doesn't support std::to_chars for floating-point values :(

		auto TimePart = [](float elapsedTime) -> std::string
		{
			std::stringstream ss;
			ss.setf(std::ios::fixed, std::ios::floatfield);
			ss.precision(3);

			ss << '[';
			ss << elapsedTime;
			ss << ']';

			return ss.str();
		};
#endif
		switch (context.side)
		{
			case LogSide::Irrelevant:
				content = TimePart(context.elapsedTime) + content;
				break;

			case LogSide::Client:
				content = TimePart(context.elapsedTime) + " [C] " + content;
				break;

			case LogSide::Editor:
				content = TimePart(context.elapsedTime) + " [E] " + content;
				break;

			case LogSide::Server:
				content = TimePart(context.elapsedTime) + " [S] " + content;
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
