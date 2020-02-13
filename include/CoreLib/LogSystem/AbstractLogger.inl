// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/AbstractLogger.hpp>

namespace bw
{
	inline AbstractLogger::AbstractLogger(LogSide logSide) :
	m_side(logSide)
	{
	}

	inline LogSide AbstractLogger::GetSide() const
	{
		return m_side;
	}
}
