// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/LoggerProxy.hpp>
#include <CoreLib/LogSystem/EntityLogContext.hpp>

namespace bw
{
	inline LoggerProxy::LoggerProxy(const Logger& logParent) :
	AbstractLogger(logParent.GetSide()),
	m_logParent(logParent)
	{
	}

	template<typename... Args>
	void LoggerProxy::LogFormat(const LogContext& context, Args&&... args) const
	{
		Log(context, fmt::format(std::forward<Args>(args)...));
	}

	inline LogContextPtr LoggerProxy::PushContext() const
	{
		LogContextPtr contextPtr = m_logParent.PushCustomContext<EntityLogContext>();
		InitializeContext(*contextPtr);

		return contextPtr;
	}
}
