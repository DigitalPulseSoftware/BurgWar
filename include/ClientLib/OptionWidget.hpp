// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_OPTIONMENU_HPP
#define BURGWAR_CLIENTLIB_OPTIONMENU_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Widgets/ButtonWidget.hpp>
#include <NDK/Widgets/CheckboxWidget.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <variant>

namespace bw
{
	class ConfigFile;

	class OptionWidget : public Ndk::BaseWidget
	{
		public:
			OptionWidget(Ndk::BaseWidget* parent, ConfigFile& playerConfig);
			OptionWidget(const OptionWidget&) = delete;
			OptionWidget(OptionWidget&&) = delete;
			~OptionWidget() = default;

			OptionWidget& operator=(const OptionWidget&) = delete;
			OptionWidget& operator=(OptionWidget&&) = delete;

			NazaraSignal(OnBackButtonTriggered, OptionWidget* /*optionWidget*/);

		private:
			struct Option;

			void AddSection(std::string sectionName);
			void AddBoolOption(std::string keyName, const std::string_view& label);
			void AddFloatOption(std::string keyName, const std::string_view& label);
			void AddIntegerOption(std::string keyName, const std::string_view& label);
			void AddStringOption(std::string keyName, const std::string_view& label);

			void ApplyOptions();

			void InitOption(Option& option, std::string keyName, const std::string_view& label);
			void Layout() override;

			void PostInit();
			void LoadConfigValues();

			void OnApply();
			void OnBack();
			void OnReset();

			void ShowChildren(bool show) override;

			void UpdateSection(const std::string& sectionName);

			struct Option
			{
				std::string keyName;
				Ndk::LabelWidget* label;
			};

			struct BoolOption : Option
			{
				Ndk::CheckboxWidget* optionWidget;

				NazaraSlot(Ndk::CheckboxWidget, OnStateChanged, onStateChangeSlot);
			};

			struct FloatOption : Option
			{
				Ndk::TextAreaWidget* optionWidget;

				NazaraSlot(Ndk::TextAreaWidget, OnTextChanged, onTextChangedSlot);
			};

			struct IntegerOption : Option
			{
				Ndk::TextAreaWidget* optionWidget;

				NazaraSlot(Ndk::TextAreaWidget, OnTextChanged, onTextChangedSlot);
			};

			struct StringOption : Option
			{
				Ndk::TextAreaWidget* optionWidget;

				NazaraSlot(Ndk::TextAreaWidget, OnTextChanged, onTextChangedSlot);
			};

			using OptionType = std::variant<BoolOption, FloatOption, IntegerOption, StringOption>;
			using OptionValue = std::variant<bool, double, long long, std::string>;

			struct Section
			{
				Ndk::ButtonWidget* button;
				std::vector<OptionType> options;

				NazaraSlot(Ndk::ButtonWidget, OnButtonTrigger, onTriggerSlot);
			};

			std::unordered_map<std::string /*section*/, Section> m_sections;
			std::unordered_map<std::string /*optionName*/, OptionValue> m_updatedValues;
			Section* m_activeSection;
			Ndk::ButtonWidget* m_applyButton;
			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_resetButton;
			ConfigFile& m_playerConfig;
			bool m_ignoreWidgetUpdate;
	};
}

#include <ClientLib/OptionWidget.inl>

#endif
