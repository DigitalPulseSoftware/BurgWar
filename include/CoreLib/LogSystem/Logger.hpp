// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP

#include <CoreLib/LogSystem/AbstractLogger.hpp>
#include <CoreLib/LogSystem/Enums.hpp>
#include <CoreLib/LogSystem/LogContext.hpp>
#include <CoreLib/LogSystem/LogContextPtr.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <fmt/format.h>
#include <memory>
#include <vector>

#define bwLog(logObject, lvl, ...) do \
{ \
	auto _bwLogContext = (logObject).PushContext(); \
	_bwLogContext->level = lvl; \
	if ((logObject).ShouldLog(*_bwLogContext)) \
	{ \
		(logObject).InitializeContext(*_bwLogContext); \
		(logObject).LogFormat(*_bwLogContext, __VA_ARGS__); \
	} \
} \
while (false)

namespace bw
{
	class BurgApp;
	class LogSink;

	class Logger : public AbstractLogger
	{
		friend class LogContextPtr;

		public:
			inline Logger(BurgApp& app, LogSide logSide, std::size_t contextSize = sizeof(bw::LogContext));
			inline Logger(BurgApp& app, LogSide logSide, const AbstractLogger& logParent, std::size_t contextSize = sizeof(bw::LogContext));
			Logger(const Logger&) = delete;
			Logger(Logger&&) noexcept = default;
			~Logger() = default;

			template<typename... Args> void LogFormat(const LogContext& context, Args&& ... args) const;

			void Log(const LogContext& context, std::string content) const override;
			void LogRaw(const LogContext& context, std::string_view content) const override;

			virtual void InitializeContext(LogContext& context) const;

			inline LogContextPtr PushContext() const;

			inline void RegisterSink(std::shared_ptr<LogSink> sinkPtr);

			inline void SetMinimumLogLevel(LogLevel level);

			bool ShouldLog(const LogContext& context) const override;

			Logger& operator=(const Logger& logger) = delete;
			Logger& operator=(Logger&&) noexcept = default;

		protected:
			template<typename T> T* AllocateContext(Nz::MemoryPool& pool) const;
			virtual LogContext* NewContext(Nz::MemoryPool& pool) const;
			virtual void OverrideContent(const LogContext& context, std::string& content) const;

		private:
			void FreeContext(LogContext* context) const;

			mutable Nz::MemoryPool m_contextPool;
			BurgApp& m_app;
			LogLevel m_minimumLogLevel;
			Nz::MovablePtr<const AbstractLogger> m_logParent;
			std::vector<std::shared_ptr<LogSink>> m_sinks;
	};
}

#include <CoreLib/LogSystem/Logger.inl>

#endif
