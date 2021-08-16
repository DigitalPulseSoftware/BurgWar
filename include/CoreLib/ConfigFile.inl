// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/ConfigFile.hpp>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace bw
{
	inline ConfigFile::ConfigFile(BurgApp& app) :
	m_app(app)
	{
	}

	inline bool ConfigFile::GetBoolValue(const std::string& optionName) const
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return std::get<BoolOption>(m_options[optionIndex].data).value;
	}

	template<typename T>
	T ConfigFile::GetFloatValue(const std::string& optionName) const
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return static_cast<T>(std::get<FloatOption>(m_options[optionIndex].data).value);
	}

	template<typename T>
	T ConfigFile::GetIntegerValue(const std::string& optionName) const
	{
		std::size_t optionIndex = GetOptionIndex(optionName);

		long long value = std::get<IntegerOption>(m_options[optionIndex].data).value;
		if constexpr (std::is_unsigned_v<T>)
		{
			if (value < 0)
				throw std::range_error(optionName + " value is smaller than T minimal representable value(" + std::to_string(value) + ')');

			unsigned long long unsignedValue = static_cast<unsigned long long>(value);
			if (unsignedValue > std::numeric_limits<T>::max())
				throw std::range_error(optionName + " value is greater than T maximal representable value (" + std::to_string(unsignedValue) + ')');

			return static_cast<T>(unsignedValue);
		}
		else
		{
			if (value < std::numeric_limits<T>::min())
				throw std::range_error(optionName + " value is smaller than T minimal representable value(" + std::to_string(value) + ')');

			if (value > std::numeric_limits<T>::max())
				throw std::range_error(optionName + " value is greater than T maximal representable value (" + std::to_string(value) + ')');

			return static_cast<T>(value);
		}
	}

	template<typename T>
	void ConfigFile::RegisterOption(std::string optionName, T&& optionData)
	{
		if (optionData.defaultValue.has_value())
			optionData.value = optionData.defaultValue.value();

		RegisterConfig(std::move(optionName), std::move(optionData));
	}

	inline std::size_t ConfigFile::GetOptionIndex(const std::string& optionName) const
	{
		auto it = m_optionByName.find(optionName);
		assert(it != m_optionByName.end());

		return it->second;
	}

	inline const std::string& ConfigFile::GetStringValue(const std::string& optionName) const
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return std::get<StringOption>(m_options[optionIndex].data).value;
	}

	inline Nz::Signal<bool>& ConfigFile::GetBoolUpdateSignal(const std::string& optionName)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return std::get<BoolOption>(m_options[optionIndex].data).OnValueUpdate;
	}

	inline Nz::Signal<double>& ConfigFile::GetFloatUpdateSignal(const std::string& optionName)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return std::get<FloatOption>(m_options[optionIndex].data).OnValueUpdate;
	}

	inline Nz::Signal<long long>& ConfigFile::GetIntegerUpdateSignal(const std::string& optionName)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return std::get<IntegerOption>(m_options[optionIndex].data).OnValueUpdate;
	}

	inline Nz::Signal<const std::string&>& ConfigFile::GetStringUpdateSignal(const std::string& optionName)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);
		return std::get<StringOption>(m_options[optionIndex].data).OnValueUpdate;
	}

	inline void ConfigFile::RegisterBoolOption(std::string optionName, std::optional<bool> defaultValue)
	{
		BoolOption boolOption;
		boolOption.defaultValue = std::move(defaultValue);

		RegisterOption(std::move(optionName), std::move(boolOption));
	}

	inline void ConfigFile::RegisterFloatOption(std::string optionName, std::optional<double> defaultValue, FloatValidation validation)
	{
		RegisterFloatOption(std::move(optionName), -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::move(defaultValue), std::move(validation));
	}

	inline void ConfigFile::RegisterFloatOption(std::string optionName, double minBounds, double maxBounds, std::optional<double> defaultValue, FloatValidation validation)
	{
		FloatOption floatOption;
		floatOption.defaultValue = std::move(defaultValue);
		floatOption.maxBounds = maxBounds;
		floatOption.minBounds = minBounds;
		floatOption.validation = std::move(validation);

		RegisterOption(std::move(optionName), std::move(floatOption));
	}

	inline void ConfigFile::RegisterIntegerOption(std::string optionName, std::optional<long long> defaultValue, IntegerValidation validation)
	{
		RegisterIntegerOption(std::move(optionName), std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max(), std::move(defaultValue), std::move(validation));
	}

	inline void ConfigFile::RegisterIntegerOption(std::string optionName, long long minBounds, long long maxBounds, std::optional<long long> defaultValue, IntegerValidation validation)
	{
		IntegerOption intOption;
		intOption.defaultValue = std::move(defaultValue);
		intOption.maxBounds = maxBounds;
		intOption.minBounds = minBounds;
		intOption.validation = std::move(validation);

		RegisterOption(std::move(optionName), std::move(intOption));
	}

	inline void ConfigFile::RegisterStringOption(std::string optionName, std::optional<std::string> defaultValue, StringValidation validation)
	{
		StringOption strOption;
		strOption.defaultValue = std::move(defaultValue);
		strOption.validation = std::move(validation);

		RegisterOption(std::move(optionName), std::move(strOption));
	}

	inline bool ConfigFile::SetBoolValue(const std::string& optionName, bool value)
	{
		std::size_t optionIndex = GetOptionIndex(optionName);

		BoolOption& option = std::get<BoolOption>(m_options[optionIndex].data);
		if (option.value != value)
		{
			option.OnValueUpdate(value);
			option.value = value;
		}

		return true;
	}
}
