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

		content = "[Match " + matchContext.match->GetName() + "@T" + std::to_string(matchContext.tick) + "] " + content;
	
		Logger::OverrideContent(context, content);
	}

	LogContext* MatchLogger::NewContext(Nz::MemoryPool& pool) const
	{
		MatchLogContext* matchLogContext = AllocateContext<MatchLogContext>(pool);
		matchLogContext->match = &m_sharedMatch;

		return matchLogContext;
	}

	void MatchLogger::InitializeContext(LogContext& context) const
	{
		Logger::InitializeContext(context);

		MatchLogContext& matchContext = static_cast<MatchLogContext&>(context);
		matchContext.tick = matchContext.match->GetCurrentTick();
	}
}
