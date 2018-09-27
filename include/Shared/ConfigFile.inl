// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/ConfigFile.hpp>
#include <Nazara/Core/Error.hpp>
#include <limits>
#include <stdexcept>

namespace bw
{
	inline bool ConfigFile::GetBoolOption(const std::string& optionName) const
	{
		return std::get<BoolOption>(m_options.at(optionName)).value;
	}

	template<typename T>
	T ConfigFile::GetFloatOption(const std::string& optionName) const
	{
		return static_cast<T>(std::get<FloatOption>(m_options.at(optionName)).value);
	}

	template<typename T>
	T ConfigFile::GetIntegerOption(const std::string& optionName) const
	{
		long long value = std::get<IntegerOption>(m_options.at(optionName)).value;
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

	inline const std::string& ConfigFile::GetStringOption(const std::string& optionName) const
	{
		return std::get<StringOption>(m_options.at(optionName)).value;
	}

	inline void ConfigFile::RegisterBoolOption(std::string optionName)
	{
		RegisterOption(std::move(optionName), BoolOption{});
	}

	inline void ConfigFile::RegisterFloatOption(std::string optionName)
	{
		RegisterFloatOption(std::move(optionName), -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
	}

	inline void ConfigFile::RegisterFloatOption(std::string optionName, double minBounds, double maxBounds)
	{
		FloatOption floatOption;
		floatOption.maxBounds = maxBounds;
		floatOption.minBounds = minBounds;

		RegisterOption(std::move(optionName), std::move(floatOption));
	}

	inline void ConfigFile::RegisterIntegerOption(std::string optionName)
	{
		RegisterIntegerOption(std::move(optionName), std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max());
	}

	inline void ConfigFile::RegisterIntegerOption(std::string optionName, long long minBounds, long long maxBounds)
	{
		IntegerOption intOption;
		intOption.maxBounds = maxBounds;
		intOption.minBounds = minBounds;

		RegisterOption(std::move(optionName), std::move(intOption));
	}

	inline void ConfigFile::RegisterStringOption(std::string optionName)
	{
		RegisterOption(std::move(optionName), StringOption{});
	}

	inline void ConfigFile::SetBoolOption(const std::string& optionName, bool value)
	{
		auto it = m_options.find(optionName);
		NazaraAssert(it != m_options.end(), "Options does not exist");

		std::get<BoolOption>(it->second).value = value;
	}

	inline void ConfigFile::SetFloatOption(const std::string & optionName, double value)
	{
		auto it = m_options.find(optionName);
		NazaraAssert(it != m_options.end(), "Options does not exist");

		std::get<FloatOption>(it->second).value = value;
	}

	inline void ConfigFile::SetIntegerOption(const std::string & optionName, long long value)
	{
		auto it = m_options.find(optionName);
		NazaraAssert(it != m_options.end(), "Options does not exist");

		std::get<IntegerOption>(it->second).value = value;
	}

	inline void ConfigFile::SetStringOption(const std::string & optionName, std::string value)
	{
		auto it = m_options.find(optionName);
		NazaraAssert(it != m_options.end(), "Options does not exist");

		std::get<StringOption>(it->second).value = value;
	}

	inline void ConfigFile::RegisterOption(std::string optionName, ConfigOption option)
	{
		NazaraAssert(m_options.find(optionName) == m_options.end(), "Option already exists");

		m_options.emplace(std::move(optionName), std::move(option));
	}
}
