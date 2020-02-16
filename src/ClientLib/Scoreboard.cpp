// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scoreboard.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <Nazara/Utility/Font.hpp>
#include <NDK/Widgets.hpp>
#include "..\..\include\ClientLib\Scoreboard.hpp"

namespace bw
{
	Scoreboard::Scoreboard(Ndk::BaseWidget* parent, const Logger& logger) :
	BaseWidget(parent),
	m_logger(logger)
	{
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
		Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		std::size_t columnIndex = m_columns.size();

		auto& columnData = m_columns.emplace_back();
		columnData.name = std::move(name);
		columnData.widget = Add<Ndk::LabelWidget>();
		columnData.widget->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, columnData.name, 24, 0));

		Layout();

		return columnIndex;
	}

	std::size_t Scoreboard::AddTeam(std::string name, Nz::Color color)
	{
		Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		std::size_t teamIndex = m_teams.size();

		auto& teamData = m_teams.emplace_back();
		teamData.color = color;
		teamData.name = std::move(name);

		teamData.background = Add<Ndk::BaseWidget>();
		teamData.widget = Add<Ndk::LabelWidget>();
		teamData.widget->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, teamData.name, 36, Nz::TextStyle_Bold));

		Layout();

		return teamIndex;
	}

	void Scoreboard::RegisterPlayer(std::size_t playerIndex, std::size_t teamId, std::vector<std::string> values)
	{
		if (m_players.size() <= playerIndex)
			m_players.resize(playerIndex + 1);

		auto& playerData = m_players[playerIndex].emplace();
		playerData.teamId = teamId;

		Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		for (std::string& value : values)
		{
			auto& columnData = playerData.values.emplace_back();
			columnData.value = std::move(value);
			columnData.label = Add<Ndk::LabelWidget>();
			columnData.label->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, columnData.value, 18, 0));
		}

		Layout();
	}

	Nz::String Scoreboard::ToString() const
	{
		return "Scoreboard";
	}

	void Scoreboard::UnregisterPlayer(std::size_t playerIndex)
	{
		if (playerIndex >= m_players.size())
			return;

		auto& playerData = m_players[playerIndex];
		for (auto& columnData : playerData->values)
			columnData.label->Destroy();

		m_players[playerIndex].reset();

		Layout();
	}

	void Scoreboard::UpdatePlayerValue(std::size_t playerIndex, std::size_t valueIndex, std::string value)
	{
		if (playerIndex >= m_players.size() || !m_players[playerIndex].has_value())
			return;

		auto& playerData = m_players[playerIndex];
		if (valueIndex >= playerData->values.size())
			return;

		Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		auto& columnData = playerData->values[valueIndex];
		columnData.value = std::move(value);
		columnData.label->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, columnData.value, 18, 0));

		Layout();
	}

	void Scoreboard::Layout()
	{
		Nz::Vector2f size = GetSize();
		m_backgroundWidget->Resize(size);
		m_scrollArea->Resize(size);

		Nz::StackArray<float> columnOffsets = NazaraStackArrayNoInit(float, m_columns.size());

		float cursor = 0.f;
		float height = 0.f;

		for (std::size_t i = 0; i < m_columns.size(); ++i)
		{
			auto& columnData = m_columns[i];
			columnOffsets[i] = cursor;

			columnData.widget->SetPosition(cursor, 0.f);
			cursor += columnData.widget->GetWidth() + 50.f;

			height = std::max(height, columnData.widget->GetHeight());
		}

		cursor = height;
		for (auto& playerOpt : m_players)
		{
			if (!playerOpt)
				continue;

			height = 0.f;

			auto& playerData = playerOpt.value();
			for (std::size_t i = 0; i < playerData.values.size(); ++i)
			{
				auto& columnData = playerData.values[i];
				float offset = columnOffsets[i];
				columnData.label->SetPosition(offset, cursor);

				height = std::max(height, columnData.label->GetHeight());
			}

			cursor += height;
		}
	}
}
