// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/EntityLogger.hpp>

namespace bw
{
	inline EntityLogger::EntityLogger(entt::entity entity, const Logger& logParent) :
	LoggerProxy(logParent),
	m_entity(std::move(entity))
	{
	}

	inline void EntityLogger::UpdateEntity(entt::entity newEntity)
	{
		m_entity = newEntity;
	}
}
