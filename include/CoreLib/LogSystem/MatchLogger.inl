// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/MatchLogger.hpp>

namespace bw
{
	inline MatchLogger::MatchLogger(BurgApp& app, SharedMatch& sharedMatch, LogSide logSide, std::size_t contextSize) :
	Logger(app, logSide, contextSize),
	m_sharedMatch(sharedMatch)
	{
	}

	inline MatchLogger::MatchLogger(BurgApp& app, SharedMatch& sharedMatch, LogSide logSide, const AbstractLogger& logParent, std::size_t contextSize) :
	Logger(app, logSide, logParent, contextSize),
	m_sharedMatch(sharedMatch)
	{
	}
}
