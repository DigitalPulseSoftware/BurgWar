// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTPACKETREGISTRY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTPACKETREGISTRY_HPP

#include <CoreLib/LogSystem/Logger.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <tsl/hopscotch_map.h>
#include <optional>

namespace bw
{
	class ScriptHandlerRegistry
	{
		public:
			inline ScriptHandlerRegistry(Logger& logger);
			~ScriptHandlerRegistry() = default;

			template<typename... Args>
			std::optional<sol::object> Call(const std::string& name, Args&&... args) const;

			void Clear();

			inline bool Has(const std::string& name) const;

			inline void Register(std::string name, sol::main_protected_function handler);
			inline void Unregister(const std::string& name);

		private:
			tsl::hopscotch_map<std::string /*packetName*/, sol::main_protected_function /*handler*/> m_handlers;
			Logger& m_logger;
	};
}

#include <CoreLib/Scripting/ScriptHandlerRegistry.inl>

#endif
