// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CONFIGFILE_HPP
#define BURGWAR_CORELIB_CONFIGFILE_HPP

#include <Nazara/Core/Signal.hpp>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace bw
{
	class BurgApp;

	class ConfigFile
	{
		public:
			ConfigFile(BurgApp& app);
			~ConfigFile() = default;

			inline bool GetBoolValue(const std::string& optionName) const;
			template<typename T> T GetFloatValue(const std::string& optionName) const;
			template<typename T> T GetIntegerValue(const std::string& optionName) const;
			inline std::size_t GetOptionIndex(const std::string& optionName) const;
			inline const std::string& GetStringValue(const std::string& optionName) const;

			inline Nz::Signal<bool>& GetBoolUpdateSignal(const std::string& optionName);
			inline Nz::Signal<double>& GetFloatUpdateSignal(const std::string& optionName);
			inline Nz::Signal<long long>& GetIntegerUpdateSignal(const std::string& optionName);
			inline Nz::Signal<const std::string&>& GetStringUpdateSignal(const std::string& optionName);

			bool LoadFromFile(const std::filesystem::path& filePath);
			bool SaveToFile(const std::filesystem::path& filePath);

			inline bool SetBoolValue(const std::string& optionName, bool value);
			bool SetFloatValue(const std::string& optionName, double value);
			bool SetIntegerValue(const std::string& optionName, long long value);
			inline bool SetStringValue(const std::string& optionName, std::string value);

		protected:
			inline void RegisterBoolOption(std::string optionName, std::optional<bool> defaultValue = std::nullopt);
			inline void RegisterFloatOption(std::string optionName, std::optional<double> defaultValue = std::nullopt);
			inline void RegisterFloatOption(std::string optionName, double minBounds, double maxBounds, std::optional<double> defaultValue = std::nullopt);
			inline void RegisterIntegerOption(std::string optionName, std::optional<long long> defaultValue = std::nullopt);
			inline void RegisterIntegerOption(std::string optionName, long long minBounds, long long maxBounds, std::optional<long long> defaultValue = std::nullopt);
			inline void RegisterStringOption(std::string optionName, std::optional<std::string> defaultValue = std::nullopt);

		private:
			struct BoolOption
			{
				bool value;
				std::optional<bool> defaultValue;

				NazaraSignal(OnValueUpdate, bool /*newValue*/);
			};

			struct FloatOption
			{
				double maxBounds;
				double minBounds;
				double value;
				std::optional<double> defaultValue;

				NazaraSignal(OnValueUpdate, double /*newValue*/);
			};

			struct IntegerOption
			{
				long long maxBounds;
				long long minBounds;
				long long value;
				std::optional<long long> defaultValue;

				NazaraSignal(OnValueUpdate, long long /*newValue*/);
			};

			struct StringOption
			{
				std::string value;
				std::optional<std::string> defaultValue;

				NazaraSignal(OnValueUpdate, const std::string& /*newValue*/);
			};

			using ConfigData = std::variant<BoolOption, FloatOption, IntegerOption, StringOption>;

			struct ConfigOption
			{
				std::string name;
				ConfigData data;
			};

			struct ConfigSection
			{
				std::string sectionName;
				std::unordered_map<std::string, std::size_t> options;
				std::unordered_map<std::string, ConfigSection> subsections;
			};

			template<typename T> void RegisterOption(std::string optionName, T&& optionData);
			void RegisterConfig(std::string optionName, ConfigData value);
			
			void SaveSectionToFile(std::fstream& file, const ConfigSection& section, std::size_t indentCount);

			std::vector<ConfigOption> m_options;
			std::unordered_map<std::string, ConfigSection> m_subsections;
			std::unordered_map<std::string, std::size_t> m_optionByName;
			BurgApp& m_app;
	};
}

#include <CoreLib/ConfigFile.inl>

#endif
