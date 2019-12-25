// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/EntityLogger.hpp>
#include <CoreLib/LogSystem/MatchLogger.hpp>

namespace bw
{
	void EntityLogger::OverrideContent(const LogContext& context, std::string& content) const
	{
		const EntityLogContext& entityContext = static_cast<const EntityLogContext&>(context);

		if (entityContext.entity)
			content = "[Entity " + std::to_string(entityContext.entity->GetId()) + "] " + content;
		else
			content = "[Entity <invalid>] " + content;

		LoggerProxy::OverrideContent(context, content);
	}

	void EntityLogger::InitializeContext(LogContext& context) const
	{
		LoggerProxy::InitializeContext(context);

		EntityLogContext& matchContext = static_cast<EntityLogContext&>(context);
		matchContext.entity = m_entity;
	}
}
