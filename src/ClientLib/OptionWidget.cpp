// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/OptionWidget.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>

namespace bw
{
	namespace
	{
		constexpr float optionPadding = 5.f;
		constexpr float optionSpace = 10.f;
	}

	OptionWidget::OptionWidget(Ndk::BaseWidget* parent, ConfigFile& playerConfig) :
	BaseWidget(parent),
	m_activeSection(nullptr),
	m_playerConfig(playerConfig),
	m_ignoreWidgetUpdate(false)
	{
		AddSection("Player");
		{
			AddStringOption("Player.Name", "Player name");
		}

		AddSection("Sound");
		{
			AddIntegerOption("Sound.GlobalVolume", "Global volume");
			AddIntegerOption("Sound.MusicVolume",  "Music volume");
			AddIntegerOption("Sound.EffectVolume", "Effect volume");
		}

		LoadConfigValues();

		m_applyButton = Add<Ndk::ButtonWidget>();
		m_applyButton->UpdateText(Nz::SimpleTextDrawer::Draw("Apply", 24));
		m_applyButton->Resize(m_applyButton->GetPreferredSize());
		m_applyButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnApply();
		});
		
		m_backButton = Add<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->Resize(m_backButton->GetPreferredSize());
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBack();
		});
		
		m_resetButton = Add<Ndk::ButtonWidget>();
		m_resetButton->UpdateText(Nz::SimpleTextDrawer::Draw("Reset", 24));
		m_resetButton->Resize(m_resetButton->GetPreferredSize());
		m_resetButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnReset();
		});

		PostInit();
		UpdateSection("Player"); //< Default to first section
	}
	
	void OptionWidget::AddSection(std::string sectionName)
	{
		assert(m_sections.find(sectionName) == m_sections.end());

		float cursor = 0.f;
		for (auto&& [_, sectionData] : m_sections)
			cursor += sectionData.button->GetHeight() + 5.f;

		auto it = m_sections.emplace(std::move(sectionName), Section{}).first;
		Section& section = it->second;
		section.button = Add<Ndk::ButtonWidget>();
		section.button->UpdateText(Nz::SimpleTextDrawer::Draw(it->first, 36));
		section.button->Resize(section.button->GetPreferredSize());
		section.button->SetPosition(0.f, cursor);

		section.onTriggerSlot.Connect(section.button->OnButtonTrigger, [this, sectionName = it->first](const Ndk::ButtonWidget*)
		{
			UpdateSection(sectionName);
		});

		UpdateSection(it->first);
	}
	
	void OptionWidget::AddBoolOption(std::string keyName, const std::string_view& label)
	{
		BoolOption option;
		InitOption(option, std::move(keyName), label);

		option.optionWidget = Add<Ndk::CheckboxWidget>();
		option.optionWidget->Resize(option.optionWidget->GetPreferredSize());

		option.onStateChangeSlot.Connect(option.optionWidget->OnStateChanged, [this, keyName = option.keyName](const Ndk::CheckboxWidget* checkbox)
		{
			m_updatedValues[keyName] = checkbox->GetState() == Ndk::CheckboxState_Checked;
		});

		m_activeSection->options.emplace_back(std::move(option));
	}

	void OptionWidget::AddFloatOption(std::string keyName, const std::string_view& label)
	{
		FloatOption option;
		InitOption(option, std::move(keyName), label);

		option.optionWidget = Add<Ndk::TextAreaWidget>();
		option.optionWidget->Resize({ 50.f, option.optionWidget->GetPreferredHeight() });
		option.optionWidget->SetTextColor(Nz::Color::Black);

		option.optionWidget->SetCharacterFilter([](char32_t character)
		{
			return (character >= U'0' && character <= U'9') || (character == U'.') || (character == U'-');
		});

		option.onTextChangedSlot.Connect(option.optionWidget->OnTextChanged, [this, keyName = option.keyName, textArea = option.optionWidget](const Ndk::AbstractTextAreaWidget* /*textArea*/, const Nz::String& text)
		{
			if (m_ignoreWidgetUpdate)
				return;

			double value;
			if (text.ToDouble(&value))
				m_updatedValues[keyName] = value;
			else
			{
				// Revert to config value
				m_ignoreWidgetUpdate = true;
				textArea->SetText(Nz::String::Number(m_playerConfig.GetFloatValue<double>(keyName)));
				m_ignoreWidgetUpdate = false;
			}
		});

		m_activeSection->options.emplace_back(std::move(option));
	}

	void OptionWidget::AddIntegerOption(std::string keyName, const std::string_view& label)
	{
		IntegerOption option;
		InitOption(option, std::move(keyName), label);

		option.optionWidget = Add<Ndk::TextAreaWidget>();
		option.optionWidget->Resize({ 50.f, option.optionWidget->GetPreferredHeight() });
		option.optionWidget->SetTextColor(Nz::Color::Black);
		
		option.optionWidget->SetCharacterFilter([](char32_t character)
		{
			return (character >= U'0' && character <= U'9') || (character == U'-');
		});

		option.onTextChangedSlot.Connect(option.optionWidget->OnTextChanged, [this, keyName = option.keyName, textArea = option.optionWidget](const Ndk::AbstractTextAreaWidget* /*textArea*/, const Nz::String& text)
		{
			if (m_ignoreWidgetUpdate)
				return;

			long long value;
			if (text.ToInteger(&value))
				m_updatedValues[keyName] = value;
			else
			{
				// Revert to config value
				m_ignoreWidgetUpdate = true;
				textArea->SetText(Nz::String::Number(m_playerConfig.GetIntegerValue<long long>(keyName)));
				m_ignoreWidgetUpdate = false;
			}
		});

		m_activeSection->options.emplace_back(std::move(option));
	}

	void OptionWidget::AddStringOption(std::string keyName, const std::string_view& label)
	{
		StringOption option;
		InitOption(option, std::move(keyName), label);

		option.optionWidget = Add<Ndk::TextAreaWidget>();
		option.optionWidget->Resize({ 200.f, option.optionWidget->GetPreferredHeight() });
		option.optionWidget->SetTextColor(Nz::Color::Black);
		
		option.onTextChangedSlot.Connect(option.optionWidget->OnTextChanged, [this, keyName = option.keyName, textArea = option.optionWidget](const Ndk::AbstractTextAreaWidget* /*textArea*/, const Nz::String& text)
		{
			if (m_ignoreWidgetUpdate)
				return;

			m_updatedValues[keyName] = text.ToStdString();
		});

		m_activeSection->options.emplace_back(std::move(option));
	}

	void OptionWidget::ApplyOptions()
	{
		for (auto&& [optionName, optionValue] : m_updatedValues)
		{
			std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, bool>)
					m_playerConfig.SetBoolValue(optionName, arg);
				else if constexpr (std::is_same_v<T, double>)
					m_playerConfig.SetFloatValue(optionName, arg);
				else if constexpr (std::is_same_v<T, long long>)
					m_playerConfig.SetIntegerValue(optionName, arg);
				else if constexpr (std::is_same_v<T, std::string>)
					m_playerConfig.SetStringValue(optionName, arg);
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, optionValue);
		}
		m_updatedValues.clear();
	}
	
	void OptionWidget::InitOption(Option& option, std::string keyName, const std::string_view& label)
	{
		option.keyName = std::move(keyName);

		option.label = Add<Ndk::LabelWidget>();
		option.label->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(label), 24)); //< FIXME: Nazara-next
		option.label->Resize(option.label->GetPreferredSize());
	}

	void OptionWidget::Layout()
	{
		BaseWidget::Layout();

		Nz::Vector2f size = GetSize();

		for (auto&& [_, section] : m_sections)
		{
			float cursor = 0.f;
			for (OptionType& option : section.options)
			{
				std::visit([&](auto&& arg)
				{
					arg.label->SetPosition(size.x - arg.label->GetWidth() - arg.optionWidget->GetWidth() - optionSpace, cursor);
					arg.optionWidget->SetPosition(size.x - arg.optionWidget->GetWidth(), cursor);
					
					cursor += std::max(arg.label->GetHeight(), arg.optionWidget->GetHeight()) + optionPadding;
				}, option);
			}
		}

		float cursor = size.x;
		for (Ndk::ButtonWidget* button : { m_applyButton, m_resetButton, m_backButton })
		{
			button->SetPosition(cursor - button->GetWidth(), size.y - button->GetHeight());
			cursor -= button->GetWidth() + 10.f;
		}
	}

	void OptionWidget::OnApply()
	{
		ApplyOptions();

		// Reload config values in case some where invalid
		LoadConfigValues();
	}

	void OptionWidget::OnBack()
	{
		ApplyOptions();

		OnBackButtonTriggered(this);
	}

	void OptionWidget::OnReset()
	{
		LoadConfigValues();
	}

	void OptionWidget::PostInit()
	{
		Nz::Vector2f minSize = Nz::Vector2f::Zero();

		float maxSectionWidth = 0.f;
		for (auto&& [_, section] : m_sections)
		{
			maxSectionWidth = std::max(maxSectionWidth, section.button->GetWidth());
			minSize.y += std::max(minSize.y, section.button->GetPosition().y + section.button->GetHeight());

			float maxLabelWidth = 0.f;
			float maxOptionWidth = 0.f;
			for (OptionType& option : section.options)
			{
				std::visit([&](auto&& arg)
				{
					maxLabelWidth = std::max(maxLabelWidth, arg.label->GetWidth());
					maxOptionWidth = std::max(maxOptionWidth, arg.optionWidget->GetWidth());
				}, option);
			}
			maxLabelWidth += 10.f;

			minSize.x = std::max(minSize.x, maxSectionWidth + maxLabelWidth + maxOptionWidth);
		}

		maxSectionWidth += 20.f;

		float optionHeight = 0.f;

		for (auto&& [_, section] : m_sections)
		{
			float cursor = 0.f;
			for (OptionType& option : section.options)
			{
				std::visit([&](auto&& arg)
				{
					cursor += std::max(arg.label->GetHeight(), arg.optionWidget->GetHeight()) + optionPadding;
				}, option);
			}

			optionHeight += cursor - optionPadding;
		}

		minSize.y = std::max(minSize.y, optionHeight) + std::max(m_applyButton->GetHeight(), m_resetButton->GetHeight());

		SetMinimumSize(minSize);
		SetPreferredSize(minSize);
		SetMaximumSize(minSize * Nz::Vector2f(1.5f, 2.f));
	}

	void OptionWidget::LoadConfigValues()
	{
		m_ignoreWidgetUpdate = true;

		for (auto&& [_, section] : m_sections)
		{
			for (OptionType& option : section.options)
			{
				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;

					if constexpr (std::is_same_v<T, BoolOption>)
						arg.optionWidget->SetState((m_playerConfig.GetBoolValue(arg.keyName)) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
					else if constexpr (std::is_same_v<T, FloatOption>)
						arg.optionWidget->SetText(Nz::String::Number(m_playerConfig.GetFloatValue<double>(arg.keyName)));
					else if constexpr (std::is_same_v<T, IntegerOption>)
						arg.optionWidget->SetText(Nz::String::Number(m_playerConfig.GetIntegerValue<long long>(arg.keyName)));
					else if constexpr (std::is_same_v<T, StringOption>)
						arg.optionWidget->SetText(m_playerConfig.GetStringValue(arg.keyName));
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, option);
			}
		}

		m_ignoreWidgetUpdate = false;
	}

	void OptionWidget::ShowChildren(bool show)
	{
		m_applyButton->Show(show);
		m_backButton->Show(show);
		m_resetButton->Show(show);

		for (auto&& [sectionName, sectionData] : m_sections)
			sectionData.button->Show(show);

		for (OptionType& option : m_activeSection->options)
		{
			std::visit([&](auto&& arg)
			{
				arg.label->Show(show);
				arg.optionWidget->Show(show);
			}, option);
		}
	}
	
	void OptionWidget::UpdateSection(const std::string& sectionName)
	{
		auto it = m_sections.find(sectionName);
		assert(it != m_sections.end());

		// Hide previous widgets
		if (m_activeSection)
		{
			for (OptionType& option : m_activeSection->options)
			{
				std::visit([](auto&& arg)
				{
					arg.label->Hide();
					arg.optionWidget->Hide();
				}, option);
			}
		}

		m_activeSection = &it->second;

		// Show current widgets
		for (OptionType& option : m_activeSection->options)
		{
			std::visit([](auto&& arg)
			{
				arg.label->Show();
				arg.optionWidget->Show();
			}, option);
		}
	}
}
