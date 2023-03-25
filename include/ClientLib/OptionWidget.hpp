// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_OPTIONMENU_HPP
#define BURGWAR_CLIENTLIB_OPTIONMENU_HPP

#include <ClientLib/Export.hpp>
#include <NazaraUtils/Signal.hpp>
#include <Nazara/Widgets/Canvas.hpp>
#include <Nazara/Widgets/ButtonWidget.hpp>
#include <Nazara/Widgets/CheckboxWidget.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>
#include <Nazara/Widgets/TextAreaWidget.hpp>
#include <variant>

namespace bw
{
	class ConfigFile;

	class BURGWAR_CLIENTLIB_API OptionWidget : public Nz::BaseWidget
	{
		public:
			OptionWidget(Nz::BaseWidget* parent, ConfigFile& playerConfig);
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
				Nz::LabelWidget* label;
			};

			struct BoolOption : Option
			{
				Nz::CheckboxWidget* optionWidget;

				NazaraSlot(Nz::CheckboxWidget, OnCheckboxStateUpdate, onStateChangeSlot);
			};

			struct FloatOption : Option
			{
				Nz::TextAreaWidget* optionWidget;

				NazaraSlot(Nz::TextAreaWidget, OnTextChanged, onTextChangedSlot);
			};

			struct IntegerOption : Option
			{
				Nz::TextAreaWidget* optionWidget;

				NazaraSlot(Nz::TextAreaWidget, OnTextChanged, onTextChangedSlot);
			};

			struct StringOption : Option
			{
				Nz::TextAreaWidget* optionWidget;

				NazaraSlot(Nz::TextAreaWidget, OnTextChanged, onTextChangedSlot);
			};

			using OptionType = std::variant<BoolOption, FloatOption, IntegerOption, StringOption>;
			using OptionValue = std::variant<bool, double, long long, std::string>;

			struct Section
			{
				Nz::ButtonWidget* button;
				std::vector<OptionType> options;

				NazaraSlot(Nz::ButtonWidget, OnButtonTrigger, onTriggerSlot);
			};

			std::unordered_map<std::string /*section*/, Section> m_sections;
			std::unordered_map<std::string /*optionName*/, OptionValue> m_updatedValues;
			Section* m_activeSection;
			Nz::ButtonWidget* m_applyButton;
			Nz::ButtonWidget* m_backButton;
			Nz::ButtonWidget* m_resetButton;
			ConfigFile& m_playerConfig;
			bool m_ignoreWidgetUpdate;
	};
}

#include <ClientLib/OptionWidget.inl>

#endif
