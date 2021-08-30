// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/ConfigFile.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Utils.hpp>
#include <sol/sol.hpp>
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
			sol::environment configEnv(lua, sol::create, lua.globals());

			if (auto result = lua.safe_script_file(path, configEnv, sol::load_mode::text); !result.valid())
			{
				sol::error err = result;
				bwLog(m_app.GetLogger(), LogLevel::Error, "failed to load config {0}: {1}", path, err.what());
				return false;
			}

			bool hasError = false;

			Nz::Bitset<> missingOptions(m_options.size(), true);
			if (!LoadSection(configEnv, m_globalSection, missingOptions))
				hasError = true;

			for (std::size_t i = missingOptions.FindFirst(); i != missingOptions.npos; i = missingOptions.FindNext(i))
			{
				ConfigOption& option = m_options[i];

				bool hasDefaultDefault = std::visit([](auto&& arg)
				{
					return arg.defaultValue.has_value();
				}, option.data);

				if (!hasDefaultDefault)
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "missing config option \"{0}\"", option.name);
					hasError = true;
				}
				else
				{
					std::visit([&](auto&& arg)
					{
						bwLog(m_app.GetLogger(), LogLevel::Debug, "option \"{0}\" has no value, setting it to default value ({1})...", option.name, arg.defaultValue.value());
					}, option.data);
				}
			}

			if (hasError)
				return false;
		}
		catch (const sol::error& e)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "failed to parse config \"{0}\": {1}", path, e.what());
			return false;
		}

		return true;
	}

	bool ConfigFile::SaveToFile(const std::filesystem::path& filePath)
	{
		std::fstream file(filePath, std::ios::out | std::ios::trunc);
		if (!file.is_open())
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "failed to open config file {0}: {1}", filePath.generic_u8string());
			return false;
		}

		SaveSectionToFile(file, m_globalSection, 0);

		return file.good();
	}

	bool ConfigFile::LoadSection(const sol::table& table, ConfigSection& section, Nz::Bitset<>& missingOptions, const std::string& prefix)
	{
		bool hasError = false;

		table.for_each([&](const sol::object& key, const sol::object& value)
		{
			std::string keyName = key.as<std::string>();

			if (auto sectionIt = section.subsections.find(keyName); sectionIt != section.subsections.end())
			{
				if (!value.is<sol::table>())
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "\"{0}{1}\" is a subsection and was expected to be a table (got {2})", prefix, keyName, sol::type_name(table.lua_state(), value.get_type()));
					return;
				}

				if (!LoadSection(value.as<sol::table>(), *sectionIt->second, missingOptions, prefix + keyName + "."))
					hasError = true;
			}
			else if (auto optionIt = section.options.find(keyName); optionIt != section.options.end())
			{
				ConfigOption& option = m_options[optionIt->second];

				bool hasDefaultDefault = std::visit([](auto&& arg)
				{
					return arg.defaultValue.has_value();
				}, option.data);

				try
				{
					std::visit([&](auto&& option)
					{
						using T = std::decay_t<decltype(option)>;
						using ArgType = std::decay_t<decltype(option.value)>;

						ArgType optionValue;

						// Get value
						if constexpr (std::is_same_v<T, BoolOption>)
						{
							if (!value.is<bool>())
								throw std::runtime_error("expected boolean, got " + sol::type_name(table.lua_state(), value.get_type()));

							optionValue = value.as<bool>();
						}
						else if constexpr (std::is_same_v<T, FloatOption>)
						{
							if (!value.is<double>())
								throw std::runtime_error("expected number, got " + sol::type_name(table.lua_state(), value.get_type()));

							optionValue = value.as<double>();
						}
						else if constexpr (std::is_same_v<T, IntegerOption>)
						{
							if (!value.is<long long>())
								throw std::runtime_error("expected number, got " + sol::type_name(table.lua_state(), value.get_type()));

							optionValue = value.as<long long>();
						}
						else if constexpr (std::is_same_v<T, StringOption>)
						{
							if (!value.is<std::string>())
								throw std::runtime_error("expected string, got " + sol::type_name(table.lua_state(), value.get_type()));

							optionValue = value.as<std::string>();
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

						// Check bounds
						if constexpr (std::is_same_v<T, FloatOption> || std::is_same_v<T, IntegerOption>)
						{
							if (optionValue < option.minBounds)
								throw std::runtime_error("option value is under bounds (" + std::to_string(optionValue) + " < " + std::to_string(option.minBounds) + ')');
							else if (optionValue > option.maxBounds)
								throw std::runtime_error("option value is over bounds (" + std::to_string(optionValue) + " > " + std::to_string(option.maxBounds) + ')');
						}

						if constexpr (!std::is_same_v<T, BoolOption>)
						{
							if (option.validation)
							{
								auto valueOrErr = option.validation(std::move(optionValue));
								if (!valueOrErr)
									throw std::runtime_error("option value failed validation: " + valueOrErr.error());

								optionValue = std::move(valueOrErr).value();
							}
						}

						option.value = std::move(optionValue);

					}, option.data);

					missingOptions.Reset(option.index);
				}
				catch (const std::exception& e)
				{
					bwLog(m_app.GetLogger(), LogLevel::Error, "failed to load \"{0}{1}\": {2}", prefix, option.name, e.what());

					if (!hasDefaultDefault)
					{
						hasError = true;
						return;
					}
				}
			}
			else
			{
				bwLog(m_app.GetLogger(), LogLevel::Warning, "unknown config section \"{0}{1}\"", prefix, keyName);
				return;
			}
		});

		return hasError;
	}

	void ConfigFile::RegisterConfig(std::string optionName, ConfigData data)
	{
		NazaraAssert(m_optionByName.find(optionName) == m_optionByName.end(), "Option already exists");

		std::size_t optionIndex = m_options.size();
		auto& option = m_options.emplace_back();
		option.name = optionName;
		option.data = std::move(data);
		option.index = optionIndex;

		m_optionByName.emplace(std::move(optionName), optionIndex);

		ConfigSection* section = nullptr;

		ForEachSection(option.name, [&](std::string_view sectionName)
		{
			std::string name(sectionName);

			auto& subsections = (section) ? section->subsections : m_globalSection.subsections;

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
				section = &m_globalSection;

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

		bool isGlobalSection = section.sectionName.empty();
		for (auto&& [sectionName, sectionData] : section.subsections)
		{
			file << indent << sectionName << " = {\n";
			SaveSectionToFile(file, *sectionData, indentCount + 1);
			file << indent << "}";
			if (!isGlobalSection)
				file << ",";

			file << "\n";
		}
	}

	bool ConfigFile::SetFloatValue(const std::string& optionName, double value)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);

		FloatOption& option = std::get<FloatOption>(m_options[optionIndex].data);
		if (value > option.maxBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) is too big (max: {2})", m_options[optionIndex].name, value, option.maxBounds);
			return false;
		}

		if (value < option.minBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) is too small (min: {2})", m_options[optionIndex].name, value, option.minBounds);
			return false;
		}

		if (option.validation)
		{
			auto valueOrErr = option.validation(value);
			if (!valueOrErr)
			{
				bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) failed validation: {2}", m_options[optionIndex].name, value, valueOrErr.error());
				return false;
			}

			value = valueOrErr.value();
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
			bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) is too big (max: {2})", m_options[optionIndex].name, value, option.maxBounds);
			return false;
		}

		if (value < option.minBounds)
		{
			bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) is too small (min: {2})", m_options[optionIndex].name, value, option.minBounds);
			return false;
		}

		if (option.validation)
		{
			auto valueOrErr = option.validation(value);
			if (!valueOrErr)
			{
				bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) failed validation: {2}", m_options[optionIndex].name, value, valueOrErr.error());
				return false;
			}

			value = valueOrErr.value();
		}

		if (option.value != value)
		{
			option.OnValueUpdate(value);
			option.value = value;
		}

		return true;
	}

	bool ConfigFile::SetStringValue(const std::string& optionName, std::string value)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);

		StringOption& option = std::get<StringOption>(m_options[optionIndex].data);

		if (option.validation)
		{
			auto valueOrErr = option.validation(value);
			if (!valueOrErr)
			{
				bwLog(m_app.GetLogger(), LogLevel::Error, "option {0} value ({1}) failed validation: {2}", m_options[optionIndex].name, value, valueOrErr.error());
				return false;
			}

			value = std::move(valueOrErr).value();
		}

		if (option.value != value)
		{
			option.OnValueUpdate(value);
			option.value = std::move(value);
		}

		return true;
	}
}
