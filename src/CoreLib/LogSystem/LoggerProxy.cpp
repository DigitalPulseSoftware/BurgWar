// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/LoggerProxy.hpp>

namespace bw
{
	void LoggerProxy::InitializeContext(LogContext& context) const
	{
		m_logParent.InitializeContext(context);
	}

	void LoggerProxy::Log(const LogContext& context, std::string content) const
	{
		OverrideContent(context, content);

		m_logParent.Log(context, content);
	}

	void LoggerProxy::LogRaw(const LogContext& context, std::string_view content) const
	{
		m_logParent.LogRaw(context, content);
	}

	bool LoggerProxy::ShouldLog(const LogContext& context) const
	{
		return m_logParent.ShouldLog(context);
	}

	void LoggerProxy::OverrideContent(const LogContext& /*context*/, std::string& /*content*/) const
	{
	}
}
