// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Utils.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	namespace
	{
		template<typename F>
		bool ForEachSubVar(const std::string_view& varName, F func)
		{
			std::size_t pos = 0;
			std::size_t previousPos = 0;
			while ((pos = varName.find('.', previousPos)) != std::string::npos)
			{
				if (!func(varName.substr(previousPos, pos - previousPos)))
					return false;

				previousPos = pos + 1;
			}

			return func(varName.substr(previousPos));
		}
	}

	bool ConfigFile::LoadFromFile(const std::string& fileName)
	{
		sol::state lua;
		lua.open_libraries();

		try
		{
			if (auto result = lua.safe_script_file(fileName); !result.valid())
			{
				sol::error err = result;
				bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to execute {0}: {1}", fileName, err.what());
				return false;
			}

			lua_State* L = lua.lua_state();

			auto PushLuaVariable = [L](const std::string& optionName)
			{
				lua_getglobal(L, "_G");

				return ForEachSubVar(optionName, [L](std::string_view variable)
				{
					lua_getfield(L, -1, std::string(variable).data());
					lua_remove(L, -2);

					return lua_isnoneornil(L, -1) == 0;
				});
			};

			for (auto& pair : m_options)
			{
				const std::string& optionName = pair.first;
				ConfigOption& optionValue = pair.second;

				if (!PushLuaVariable(optionName))
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "Missing config option \"{0}\"", optionName);
					return false;
				}

				try
				{
					std::visit([&](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;

						// Get value
						if constexpr (std::is_same_v<T, BoolOption>)
						{
							if (!lua_isboolean(L, -1))
								throw std::runtime_error("Boolean expected");

							arg.value = lua_toboolean(L, -1);
						}
						else if constexpr (std::is_same_v<T, FloatOption>)
						{
							if (!lua_isnumber(L, -1))
								throw std::runtime_error("Float expected");

							arg.value = luaL_checknumber(L, -1);
						}
						else if constexpr (std::is_same_v<T, IntegerOption>)
						{
							if (!lua_isnumber(L, -1))
								throw std::runtime_error("Integer expected");

							arg.value = luaL_checkinteger(L, -1);
						}
						else if constexpr (std::is_same_v<T, StringOption>)
						{
							if (!lua_isstring(L, -1))
								throw std::runtime_error("string expected");

							arg.value = luaL_checkstring(L, -1);
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

						// Check bounds
						if constexpr (std::is_same_v<T, FloatOption> || std::is_same_v<T, IntegerOption>)
						{
							if (arg.value < arg.minBounds)
								throw std::runtime_error("option value is under bounds (" + std::to_string(arg.value) + " < " + std::to_string(arg.minBounds) + ')');
							else if (arg.value > arg.maxBounds)
								throw std::runtime_error("option value is over bounds (" + std::to_string(arg.value) + " > " + std::to_string(arg.maxBounds) + ')');
						}

						}, optionValue);
				}
				catch (const std::exception& e)
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to get \"{0}\": {1}", optionName, e.what());
				}
				catch (...)
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to get \"{0}\": {1}", optionName, lua_tostring(L, -1));
					lua_pop(L, lua_gettop(L));
				}

				lua_pop(L, 1);
			}
		}
		catch (const sol::error& e)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to parse \"{0}\": {1}", fileName, e.what());
			return false;
		}

		return true;
	}
}
