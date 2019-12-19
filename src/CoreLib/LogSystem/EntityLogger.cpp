// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/EntityLogger.hpp>

namespace bw
{
	bool EntityLogger::ShouldLog(const LogContext& context) const
	{
		if (!Logger::ShouldLog(context))
			return false;

		return true;
	}

	void EntityLogger::UpdateEntity(Ndk::EntityHandle newEntity)
	{
		m_entity = newEntity;
	}

	void EntityLogger::OverrideContent(const LogContext& context, std::string& content) const
	{
		const EntityLogContext& entityContext = static_cast<const EntityLogContext&>(context);

		if (entityContext.entity)
			content = "[Entity " + std::to_string(entityContext.entity->GetId()) + "] " + content;
		else
			content = "[Entity <invalid>] " + content;

		Logger::OverrideContent(context, content);
	}

	LogContext* EntityLogger::AllocateContext(Nz::MemoryPool& pool) const
	{
		return pool.New<EntityLogContext>();
	}

	void EntityLogger::InitializeContext(LogContext& context) const
	{
		Logger::InitializeContext(context);

		EntityLogContext& matchContext = static_cast<EntityLogContext&>(context);
		matchContext.entity = m_entity;
	}
}
