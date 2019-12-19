// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_ENTITYLOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_ENTITYLOGGER_HPP

#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/LogSystem/EntityLogContext.hpp>

namespace bw
{
	class EntityLogger : public Logger
	{
		public:
			inline EntityLogger(Ndk::EntityHandle entity, LogSide logSide, std::size_t contextSize = sizeof(bw::EntityLogContext));
			inline EntityLogger(Ndk::EntityHandle entity, LogSide logSide, const AbstractLogger& logParent, std::size_t contextSize = sizeof(bw::EntityLogContext));
			EntityLogger(const EntityLogger&) = default;
			EntityLogger(EntityLogger&&) = default;
			~EntityLogger() = default;

			bool ShouldLog(const LogContext& context) const override;

			void UpdateEntity(Ndk::EntityHandle newEntity);

		private:
			LogContext* AllocateContext(Nz::MemoryPool& pool) const override;
			void InitializeContext(LogContext& context) const override;
			void OverrideContent(const LogContext& context, std::string& content) const override;

			Ndk::EntityHandle m_entity;
	};
}

#include <CoreLib/LogSystem/EntityLogger.inl>

#endif
