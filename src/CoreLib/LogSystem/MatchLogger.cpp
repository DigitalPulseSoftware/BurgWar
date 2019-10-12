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
		content = "[Match " + m_sharedMatch.GetName() + "] " + content;
	}
}
