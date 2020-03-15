// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/OptionWidget.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>

namespace bw
{
	OptionWidget::OptionWidget(Ndk::BaseWidget* parent, ConfigFile& playerConfig) :
	BaseWidget(parent),
	m_activeSection(nullptr),
	m_parent(parent),
	m_playerConfig(playerConfig)
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

		LayoutOptions();
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

		if (m_playerConfig.GetBoolValue(option.keyName))
			option.optionWidget->SetState(Ndk::CheckboxState_Checked);

		option.onStateChangeSlot.Connect(option.optionWidget->OnStateChanged, [this, keyName = option.keyName](const Ndk::CheckboxWidget* checkbox)
		{
			m_playerConfig.SetBoolValue(keyName, checkbox->GetState() == Ndk::CheckboxState_Checked);
		});

		m_activeSection->options.emplace_back(std::move(option));
	}

	void OptionWidget::AddFloatOption(std::string keyName, const std::string_view& label)
	{
		FloatOption option;
		InitOption(option, std::move(keyName), label);

		option.optionWidget = Add<Ndk::TextAreaWidget>();
		option.optionWidget->Resize({ 50.f, option.optionWidget->GetPreferredHeight() });

		option.optionWidget->SetText(Nz::String::Number(m_playerConfig.GetFloatValue<double>(option.keyName)));
		option.optionWidget->SetTextColor(Nz::Color::Black);

		option.optionWidget->SetCharacterFilter([](char32_t character)
		{
			return (character >= U'0' && character <= U'9') || (character == U'.') || (character == U'-');
		});

		option.onTextChangedSlot.Connect(option.optionWidget->OnTextChanged, [this, keyName = option.keyName, textArea = option.optionWidget, ignoreEvent = false](const Ndk::AbstractTextAreaWidget* /*textArea*/, const Nz::String& text) mutable
		{
			if (ignoreEvent)
				return;

			double value;
			if (!text.ToDouble(&value) || !m_playerConfig.SetFloatValue(keyName, value))
			{
				// Revert to config value
				ignoreEvent = true;
				textArea->SetText(Nz::String::Number(m_playerConfig.GetFloatValue<double>(keyName)));
				ignoreEvent = false;
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

		option.optionWidget->SetText(Nz::String::Number(m_playerConfig.GetIntegerValue<long long>(option.keyName)));
		option.optionWidget->SetTextColor(Nz::Color::Black);
		
		option.optionWidget->SetCharacterFilter([](char32_t character)
		{
			return (character >= U'0' && character <= U'9') || (character == U'-');
		});

		option.onTextChangedSlot.Connect(option.optionWidget->OnTextChanged, [this, keyName = option.keyName, textArea = option.optionWidget, ignoreEvent = false](const Ndk::AbstractTextAreaWidget* /*textArea*/, const Nz::String& text) mutable
		{
			if (ignoreEvent)
				return;

			long long value;
			if (!text.ToInteger(&value) || !m_playerConfig.SetIntegerValue(keyName, value))
			{
				// Revert to config value
				ignoreEvent = true;
				textArea->SetText(Nz::String::Number(m_playerConfig.GetIntegerValue<long long>(keyName)));
				ignoreEvent = false;
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

		option.optionWidget->SetText(m_playerConfig.GetStringValue(option.keyName));
		option.optionWidget->SetTextColor(Nz::Color::Black);
		
		option.onTextChangedSlot.Connect(option.optionWidget->OnTextChanged, [this, keyName = option.keyName, textArea = option.optionWidget, ignoreEvent = false](const Ndk::AbstractTextAreaWidget* /*textArea*/, const Nz::String& text) mutable
		{
			if (ignoreEvent)
				return;

			if (!m_playerConfig.SetStringValue(keyName, text.ToStdString()))
			{
				// Revert to config value
				ignoreEvent = true;
				textArea->SetText(Nz::String::Number(m_playerConfig.GetIntegerValue<long long>(keyName)));
				ignoreEvent = false;
			}
		});

		m_activeSection->options.emplace_back(std::move(option));
	}
	
	void OptionWidget::InitOption(Option& option, std::string keyName, const std::string_view& label)
	{
		option.keyName = std::move(keyName);

		option.label = Add<Ndk::LabelWidget>();
		option.label->UpdateText(Nz::SimpleTextDrawer::Draw(std::string(label), 24)); //< FIXME: Nazara-next
		option.label->Resize(option.label->GetPreferredSize());
	}

	void OptionWidget::LayoutOptions()
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
			constexpr float optionPadding = 5.f;
			constexpr float optionSpace = 10.f;

			float cursor = 0.f;
			for (OptionType& option : section.options)
			{
				std::visit([&](auto&& arg)
				{
					arg.label->SetPosition(minSize.x - arg.label->GetWidth() - arg.optionWidget->GetWidth() - optionSpace, cursor);
					arg.optionWidget->SetPosition(minSize.x - arg.optionWidget->GetWidth(), cursor);

					cursor += std::max(arg.label->GetHeight(), arg.optionWidget->GetHeight()) + optionPadding;
				}, option);
			}

			optionHeight += cursor - optionPadding;
		}

		minSize.y = std::max(minSize.y, optionHeight);

		SetMinimumSize(minSize);
		SetPreferredSize(minSize);
	}

	void OptionWidget::ShowChildren(bool show)
	{
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
