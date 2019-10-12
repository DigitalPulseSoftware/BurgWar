// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGCONTEXTPTR_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGCONTEXTPTR_HPP

#include <CoreLib/LogSystem/LogContext.hpp>
#include <Nazara/Core/MovablePtr.hpp>

namespace bw
{
	class Logger;

	class LogContextPtr
	{
		friend Logger;

		public:
			LogContextPtr(const LogContextPtr&) = delete;
			LogContextPtr(LogContextPtr&&) = default;
			~LogContextPtr();

			inline LogContext& operator*();
			inline LogContext* operator->();

			LogContextPtr& operator=(const LogContextPtr&) = delete;
			LogContextPtr& operator=(LogContextPtr&&) = default;

		private:
			inline LogContextPtr(const Logger* owner, LogContext* context);

			Nz::MovablePtr<const Logger> m_owner;
			Nz::MovablePtr<LogContext> m_context;
	};
}

#include <CoreLib/LogSystem/LogContextPtr.inl>

#endif
