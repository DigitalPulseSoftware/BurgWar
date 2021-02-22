// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_ENTITYLOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_ENTITYLOGGER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LogSystem/LoggerProxy.hpp>
#include <CoreLib/LogSystem/EntityLogContext.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API EntityLogger : public LoggerProxy
	{
		public:
			inline EntityLogger(Ndk::EntityHandle entity, const Logger& logParent);
			EntityLogger(const EntityLogger&) = default;
			EntityLogger(EntityLogger&&) = default;
			~EntityLogger() = default;

			void InitializeContext(LogContext& context) const override;

			inline void UpdateEntity(Ndk::EntityHandle newEntity);

		private:
			void OverrideContent(const LogContext& context, std::string& content) const override;

			Ndk::EntityHandle m_entity;
	};
}

#include <CoreLib/LogSystem/EntityLogger.inl>

#endif
