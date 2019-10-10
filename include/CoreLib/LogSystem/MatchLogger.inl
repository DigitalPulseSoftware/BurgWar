// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/MatchLogger.hpp>

namespace bw
{
	inline MatchLogger::MatchLogger(SharedMatch& sharedMatch) :
	Logger(),
	m_sharedMatch(sharedMatch)
	{
	}

	inline MatchLogger::MatchLogger(SharedMatch& sharedMatch, AbstractLogger& logger) :
	Logger(logger),
	m_sharedMatch(sharedMatch)
	{
	}
}
