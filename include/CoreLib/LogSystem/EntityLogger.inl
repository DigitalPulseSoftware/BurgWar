// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/EntityLogger.hpp>

namespace bw
{
	inline EntityLogger::EntityLogger(Ndk::EntityHandle entity, LogSide logSide, std::size_t contextSize) :
	Logger(logSide, contextSize),
	m_entity(std::move(entity))
	{
	}

	inline EntityLogger::EntityLogger(Ndk::EntityHandle entity, LogSide logSide, const AbstractLogger& logParent, std::size_t contextSize) :
	Logger(logSide, logParent, contextSize),
	m_entity(std::move(entity))
	{
	}
}
