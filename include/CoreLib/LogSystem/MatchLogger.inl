// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/MatchLogger.hpp>

namespace bw
{
	inline MatchLogger::MatchLogger(BurgAppComponent& app, SharedMatch& sharedMatch, LogSide logSide) :
	Logger(app, logSide),
	m_sharedMatch(sharedMatch)
	{
	}

	inline MatchLogger::MatchLogger(BurgAppComponent& app, SharedMatch& sharedMatch, LogSide logSide, const AbstractLogger& logParent) :
	Logger(app, logSide, logParent),
	m_sharedMatch(sharedMatch)
	{
	}
}
