// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/EntityLogger.hpp>

namespace bw
{
	inline EntityLogger::EntityLogger(Ndk::EntityHandle entity, const Logger& logParent) :
	LoggerProxy(logParent),
	m_entity(std::move(entity))
	{
	}

	inline void EntityLogger::UpdateEntity(Ndk::EntityHandle newEntity)
	{
		m_entity = newEntity;
	}
}
