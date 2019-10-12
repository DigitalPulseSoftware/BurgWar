// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/SharedMatch.hpp>

namespace bw
{
	bool MatchLogger::ShouldLog(const LogContext& context) const
	{
		if (!Logger::ShouldLog(context))
			return false;

		return true;
	}

	void MatchLogger::OverrideContent(const LogContext& context, std::string& content) const
	{
		const MatchLogContext& matchContext = static_cast<const MatchLogContext&>(context);

		content = "[Match " + matchContext.match->GetName() + "] " + content;
	
		Logger::OverrideContent(context, content);
	}

	LogContext* MatchLogger::AllocateContext(Nz::MemoryPool& pool) const
	{
		return pool.New<MatchLogContext>();
	}

	void MatchLogger::InitializeContext(LogContext& context) const
	{
		Logger::InitializeContext(context);

		MatchLogContext& matchContext = static_cast<MatchLogContext&>(context);
		matchContext.match = &m_sharedMatch;
	}
}
