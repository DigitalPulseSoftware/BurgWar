// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Utils.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <fstream>

namespace bw
{
	namespace
	{
		template<typename SectionFunc, typename VarFunc>
		bool ForEachSection(const std::string_view& varName, SectionFunc&& sectionCB, VarFunc&& varCB)
		{
			std::size_t pos = 0;
			std::size_t previousPos = 0;
			while ((pos = varName.find('.', previousPos)) != std::string::npos)
			{
				if (!sectionCB(varName.substr(previousPos, pos - previousPos)))
					return false;

				previousPos = pos + 1;
			}

			return varCB(varName.substr(previousPos));
		}

		template<typename F>
		bool ForEachSection(const std::string_view& varName, F&& func)
		{
			return ForEachSection(varName, func, func);
		}
	}

	bool ConfigFile::LoadFromFile(const std::filesystem::path& filePath)
	{
		sol::state lua;
		lua.open_libraries();

		std::string path = filePath.generic_u8string();

		try
		{
			if (auto result = lua.safe_script_file(path); !result.valid())
			{
				sol::error err = result;
				bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to load config {0}: {1}", path, err.what());
				return false;
			}

			lua_State* L = lua.lua_state();

			auto PushLuaVariable = [L](const std::string& optionName)
			{
				lua_getglobal(L, "_G");

				return ForEachSection(optionName, [L](std::string_view variable)
				{
					lua_getfield(L, -1, std::string(variable).data());
					lua_remove(L, -2);

					return lua_isnoneornil(L, -1) == 0;
				});
			};

			// TODO use sections
			for (ConfigOption& option : m_options)
			{
				bool hasDefaultDefault = std::visit([](auto&& arg)
				{
					return arg.defaultValue.has_value();
				}, option.data);

				if (!PushLuaVariable(option.name))
				{
					if (hasDefaultDefault)
						continue;

					bwLog(m_app.GetLogger(), LogLevel::Error, "Missing config option \"{0}\"", option.name);
					return false;
				}

				try
				{
					std::visit([&](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;
						using ArgType = std::decay_t<decltype(arg.value)>;

						ArgType value;

						// Get value
						if constexpr (std::is_same_v<T, BoolOption>)
						{
							if (!lua_isboolean(L, -1))
								throw std::runtime_error("Boolean expected");

							value = lua_toboolean(L, -1);
						}
						else if constexpr (std::is_same_v<T, FloatOption>)
						{
							if (!lua_isnumber(L, -1))
								throw std::runtime_error("Float expected");

							value = luaL_checknumber(L, -1);
						}
						else if constexpr (std::is_same_v<T, IntegerOption>)
						{
							if (!lua_isnumber(L, -1))
								throw std::runtime_error("Integer expected");

							value = luaL_checkinteger(L, -1);
						}
						else if constexpr (std::is_same_v<T, StringOption>)
						{
							if (!lua_isstring(L, -1))
								throw std::runtime_error("string expected");

							value = luaL_checkstring(L, -1);
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

						// Check bounds
						if constexpr (std::is_same_v<T, FloatOption> || std::is_same_v<T, IntegerOption>)
						{
							if (value < arg.minBounds)
								throw std::runtime_error("option value is under bounds (" + std::to_string(value) + " < " + std::to_string(arg.minBounds) + ')');
							else if (value > arg.maxBounds)
								throw std::runtime_error("option value is over bounds (" + std::to_string(value) + " > " + std::to_string(arg.maxBounds) + ')');
						}

						arg.value = std::move(value);

					}, option.data);
				}
				catch (const std::exception& e)
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to load \"{0}\": {1}", option.name, e.what());

					if (!hasDefaultDefault)
						return false;
				}
				catch (...)
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to load \"{0}\": {1}", option.name, lua_tostring(L, -1));
					lua_pop(L, lua_gettop(L));

					if (!hasDefaultDefault)
						return false;
				}

				lua_pop(L, 1);
			}
		}
		catch (const sol::error& e)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to parse config \"{0}\": {1}", path, e.what());
			return false;
		}

		return true;
	}

	bool ConfigFile::SaveToFile(const std::filesystem::path& filePath)
	{
		std::fstream file(filePath, std::ios::out | std::ios::trunc);
		if (!file.is_open())
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Failed to open config file {0}: {1}", filePath.generic_u8string());
			return false;
		}

		for (auto&& [sectionName, section] : m_subsections)
		{
			if (!sectionName.empty())
			{
				file << sectionName << " = {\n";
				SaveSectionToFile(file, *section, 1);
				file << "}\n";
			}
			else
				SaveSectionToFile(file, *section, 0);
		}

		return file.good();
	}

	void ConfigFile::RegisterConfig(std::string optionName, ConfigData data)
	{
		NazaraAssert(m_optionByName.find(optionName) == m_optionByName.end(), "Option already exists");

		std::size_t optionIndex = m_options.size();
		auto& option = m_options.emplace_back();
		option.name = optionName;
		option.data = std::move(data);

		m_optionByName.emplace(std::move(optionName), optionIndex);

		ConfigSection* section = nullptr;

		ForEachSection(option.name, [&](std::string_view sectionName)
		{
			std::string name(sectionName);

			auto& subsections = (section) ? section->subsections : m_subsections;

			auto it = subsections.find(name);
			if (it == subsections.end())
			{
				auto newSection = std::make_unique<ConfigSection>();
				newSection->sectionName = name;

				it = subsections.emplace(std::move(name), std::move(newSection)).first;
			}

			section = it->second.get();
			return true;
		}, 
		[&](std::string_view varName)
		{
			if (!section)
			{
				auto it = m_subsections.emplace(std::string(), std::make_unique<ConfigSection>()).first;
				section = it->second.get();
			}

			section->options.emplace(std::string(varName), optionIndex);
			return true;
		});
	}
	
	void ConfigFile::SaveSectionToFile(std::fstream& file, const ConfigSection& section, std::size_t indentCount)
	{
		std::string indent(indentCount, '\t');

		for (auto&& [optionName, optionIndex] : section.options)
		{
			file << indent << optionName << " = ";

			auto& option = m_options[optionIndex];

			std::visit([&](auto&& option)
			{
				using T = std::decay_t<decltype(option)>;

				if constexpr (std::is_same_v<T, BoolOption>)
				{
					file << ((option.value) ? "true" : "false");
				}
				else if constexpr (std::is_same_v<T, FloatOption> || 
				                   std::is_same_v<T, IntegerOption>)
				{
					file << option.value;
				}
				else if constexpr (std::is_same_v<T, StringOption>)
				{
					// Sanitize
					file << "\"" << ReplaceStr(option.value, R"(")", R"(\")") << "\"";
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, option.data);

			if (!section.sectionName.empty())
				file << ",\n";
			else
				file << "\n";
		}

		for (auto&& [sectionName, sectionData] : section.subsections)
		{
			file << indent << sectionName << " = {\n";
			SaveSectionToFile(file, *sectionData, indentCount + 1);
			file << indent << "},\n";
		}
	}

	bool ConfigFile::SetFloatValue(const std::string& optionName, double value)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);

		FloatOption& option = std::get<FloatOption>(m_options[optionIndex].data);
		if (value > option.maxBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Option {0} value ({1}) is too big (max: {2})", m_options[optionIndex].name, value, option.maxBounds);
			return false;
		}

		if (value < option.minBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Option {0} value ({1}) is too small (min: {2})", m_options[optionIndex].name, value, option.minBounds);
			return false;
		}

		if (option.value != value)
		{
			option.OnValueUpdate(value);
			option.value = value;
		}

		return true;
	}

	bool ConfigFile::SetIntegerValue(const std::string& optionName, long long value)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);

		IntegerOption& option = std::get<IntegerOption>(m_options[optionIndex].data);
		if (value > option.maxBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Option {0} value ({1}) is too big (max: {2})", m_options[optionIndex].name, value, option.maxBounds);
			return false;
		}

		if (value < option.minBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "Option {0} value ({1}) is too small (min: {2})", m_options[optionIndex].name, value, option.minBounds);
			return false;
		}

		if (option.value != value)
		{
			option.OnValueUpdate(value);
			option.value = value;
		}

		return true;
	}
}
