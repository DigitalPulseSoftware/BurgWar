// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <iostream>

namespace bw
{
	namespace
	{
		template<typename F>
		bool ForEachSubVar(const std::string& varName, F func)
		{
			std::size_t pos = 0;
			std::size_t previousPos = 0;
			while ((pos = varName.find('.', previousPos)) != std::string::npos)
			{
				//< TODO: Use string_view
				std::string token = varName.substr(previousPos, pos - previousPos);
				previousPos = pos + 1;

				if (!func(std::move(token)))
					return false;
			}

			return func(varName.substr(previousPos));
		}
	}

	bool ConfigFile::LoadFromFile(const std::string& fileName)
	{
		Nz::LuaInstance configFile;
		configFile.LoadLibraries();

		if (!configFile.ExecuteFromFile(fileName))
		{
			std::cerr << "Failed to parse " << fileName << ": " << configFile.GetLastError() << std::endl;
			return false;
		}

		auto PushLuaVariable = [&configFile](const std::string& optionName)
		{
			configFile.GetGlobal("_G");

			return ForEachSubVar(optionName, [&configFile](const std::string& variable)
			{
				configFile.GetField(variable);
				configFile.Remove(-2);

				return configFile.IsValid(-1);
			});
		};

		for (auto& pair : m_options)
		{
			const std::string& optionName = pair.first;
			ConfigOption& optionValue = pair.second;

			if (!PushLuaVariable(optionName))
			{
				std::cerr << "Missing config option \"" << optionName << "\"" << std::endl;
				return false;
			}

			try
			{
				std::visit([&](auto&& arg) {
					using T = std::decay_t<decltype(arg)>;

					// Get value
					if constexpr (std::is_same_v<T, BoolOption>)
					{
						if (!configFile.IsOfType(-1, Nz::LuaType_Boolean))
							throw std::runtime_error("Boolean expected");

						arg.value = configFile.CheckBoolean(-1);
					}
					else if constexpr (std::is_same_v<T, FloatOption>)
					{
						if (!configFile.IsOfType(-1, Nz::LuaType_Number))
							throw std::runtime_error("Float expected");

						arg.value = configFile.CheckNumber(-1);
					}
					else if constexpr (std::is_same_v<T, IntegerOption>)
					{
						if (!configFile.IsOfType(-1, Nz::LuaType_Number))
							throw std::runtime_error("Integer expected");

						arg.value = configFile.CheckInteger(-1);
					}
					else if constexpr (std::is_same_v<T, StringOption>)
					{
						if (!configFile.IsOfType(-1, Nz::LuaType_String))
							throw std::runtime_error("string expected");

						arg.value = configFile.CheckString(-1);
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

					// Check bounds
					/*if constexpr (std::is_same_v<T, FloatOption> || std::is_same_v<T, IntegerOption>)
					{
						if (arg.value < arg.minBounds)
							throw std::runtime_error("option value is under bounds (" + std::to_string(arg.value) + " < " + std::to_string(arg.minBounds) + ')');
						else if (arg.value > arg.maxBounds)
							throw std::runtime_error("option value is over bounds (" + std::to_string(arg.value) + " > " + std::to_string(arg.maxBounds) + ')');
					}*/

				}, optionValue);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to get " << optionName << ": " << e.what() << std::endl;
			}
			catch (...)
			{
				std::cerr << "Failed to get " << optionName << ": " << configFile.ToString(-1) << std::endl;
				configFile.Pop(configFile.GetStackTop());
			}

			configFile.Pop();
		}

		return true;
	}
}
