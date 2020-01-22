// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scoreboard.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Utility/Font.hpp>
#include <NDK/Widgets.hpp>

namespace bw
{
	Scoreboard::Scoreboard(Ndk::BaseWidget* parent, const Logger& logger) :
	BaseWidget(parent),
	m_logger(logger)
	{
		Nz::FontRef chatboxFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(chatboxFont);

		m_backgroundWidget = Add<Ndk::BaseWidget>();
		m_backgroundWidget->EnableBackground(true);
		m_backgroundWidget->SetBackgroundColor(Nz::Color(43, 68, 63, 200));

		m_scrollArea = Add<Ndk::ScrollAreaWidget>(m_backgroundWidget);
		m_scrollArea->EnableScrollbar(true);
	}

	Scoreboard::~Scoreboard()
	{
		m_scrollArea->Destroy();
	}

	std::size_t Scoreboard::AddColumn(std::string name)
	{
		std::size_t columnIndex = m_columns.size();

		auto& columnData = m_columns.emplace_back();
		columnData.name = std::move(name);
		columnData.widget = Add<Ndk::LabelWidget>();
		columnData.widget->UpdateText(Nz::SimpleTextDrawer::Draw(columnData.name, 24, Nz::TextStyle_Bold));

		Layout();

		return columnIndex;
	}

	std::size_t Scoreboard::AddTeam(std::string name, Nz::Color color)
	{
		std::size_t teamIndex = m_teams.size();

		auto& teamData = m_teams.emplace_back();
		teamData.color = color;
		teamData.name = std::move(name);

		teamData.background = Add<Ndk::BaseWidget>();
		teamData.widget = Add<Ndk::LabelWidget>();
		teamData.widget->UpdateText(Nz::SimpleTextDrawer::Draw(teamData.name, 36, Nz::TextStyle_Bold));

		Layout();

		return teamIndex;
	}

	Nz::String Scoreboard::ToString() const
	{
		return "Scoreboard";
	}

	void Scoreboard::Layout()
	{
		Nz::Vector2f size = GetSize();
		m_backgroundWidget->Resize(size);
		m_scrollArea->Resize(size);
	}
}
