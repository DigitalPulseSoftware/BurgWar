// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptHandlerRegistry.hpp>

namespace bw
{
	inline ScriptHandlerRegistry::ScriptHandlerRegistry(Logger& logger) :
	m_logger(logger)
	{
	}

	template<typename... Args>
	std::optional<sol::object> ScriptHandlerRegistry::Call(const std::string& name, Args&&... args) const
	{
		if (auto it = m_handlers.find(name); it != m_handlers.end())
		{
			const sol::protected_function& handler = it.value();
			if (handler)
			{
				auto result = handler(std::forward<Args>(args)...);
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(m_logger, LogLevel::Error, "\"{0}\" handler failed: {1}", name, err.what());
					return std::nullopt;
				}

				return result;
			}
			else
				return sol::nil;
		}
		else
			return sol::nil;
	}

	inline void ScriptHandlerRegistry::Clear()
	{
		m_handlers.clear();
	}

	inline bool ScriptHandlerRegistry::Has(const std::string& name) const
	{
		return m_handlers.find(name) != m_handlers.end();
	}
	
	inline void ScriptHandlerRegistry::Register(std::string name, sol::protected_function handler)
	{
		m_handlers.emplace(std::move(name), std::move(handler));
	}
	
	inline void ScriptHandlerRegistry::Unregister(const std::string& name)
	{
		m_handlers.erase(name);
	}
}
