// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scoreboard.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <Nazara/TextRenderer/Font.hpp>
#include <Nazara/TextRenderer/SimpleTextDrawer.hpp>

namespace bw
{
	Scoreboard::Scoreboard(Nz::BaseWidget* parent, const Logger& logger) :
	BaseWidget(parent),
	m_logger(logger)
	{
		m_backgroundWidget = Add<Nz::BaseWidget>();
		m_backgroundWidget->EnableBackground(true);
		m_backgroundWidget->SetBackgroundColor(Nz::Color(80, 80, 80, 200));

		m_columnBackgroundWidget = Add<Nz::BaseWidget>();
		m_columnBackgroundWidget->EnableBackground(true);
		m_columnBackgroundWidget->SetBackgroundColor(Nz::Color(20, 20, 20, 127));

		m_contentWidget = Add<Nz::BaseWidget>();
	
		m_scrollArea = Add<Nz::ScrollAreaWidget>(m_contentWidget);
		m_scrollArea->EnableScrollbar(true);
	}

	Scoreboard::~Scoreboard()
	{
		m_scrollArea->Destroy();
	}

	std::size_t Scoreboard::AddColumn(std::string name)
	{
		std::shared_ptr<Nz::Font> scoreMenuFont = Nz::Font::GetDefault();
		//Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		std::size_t columnIndex = m_columns.size();

		auto& columnData = m_columns.emplace_back();
		columnData.name = std::move(name);
		columnData.widget = Add<Nz::LabelWidget>();
		columnData.widget->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, columnData.name, 24, 0));
		columnData.widget->Resize(columnData.widget->GetPreferredSize());

		Layout();

		return columnIndex;
	}

	std::size_t Scoreboard::AddTeam(std::string name, Nz::Color color)
	{
		std::shared_ptr<Nz::Font> scoreMenuFont = Nz::Font::GetDefault();
		//Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		std::size_t teamIndex = m_teams.size();

		auto& teamData = m_teams.emplace_back();
		teamData.color = color;
		teamData.name = std::move(name);

		teamData.background = m_contentWidget->Add<Nz::BaseWidget>();

		//teamData.line = m_contentWidget->Add<Nz::ImageWidget>();
		//teamData.line->SetColor(teamData.color);

		teamData.widget = m_contentWidget->Add<Nz::LabelWidget>();
		teamData.widget->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, teamData.name, 24, 0, teamData.color));
		teamData.widget->Resize(teamData.widget->GetPreferredSize());

		Layout();

		return teamIndex;
	}

	void Scoreboard::RegisterPlayer(std::size_t playerIndex, std::size_t teamId, std::vector<std::string> values, std::optional<Nz::Color> color, bool isLocalPlayer)
	{
		if (m_players.size() <= playerIndex)
			m_players.resize(playerIndex + 1);

		UnregisterPlayer(playerIndex);

		auto& playerData = m_players[playerIndex].emplace();
		playerData.background = m_contentWidget->Add<Nz::BaseWidget>();
		playerData.background->EnableBackground(isLocalPlayer);
		playerData.background->SetBackgroundColor(Nz::Color(255, 255, 255, 60));
		playerData.color = color;
		playerData.teamId = teamId;

		Nz::Color playerColor = Nz::Color::White();
		if (playerData.color)
			playerColor = *playerData.color;
		else if (teamId < m_teams.size())
			playerColor = m_teams[teamId].color;

		std::shared_ptr<Nz::Font> scoreMenuFont = Nz::Font::GetDefault();
		//Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");
		assert(scoreMenuFont);

		for (std::string& value : values)
		{
			auto& columnData = playerData.values.emplace_back();
			columnData.value = std::move(value);
			columnData.label = m_contentWidget->Add<Nz::LabelWidget>();
			columnData.label->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, columnData.value, 18, 0, playerColor));
			columnData.label->Resize(columnData.label->GetPreferredSize());
		}

		Layout();
	}

	std::string Scoreboard::ToString() const
	{
		return "Scoreboard";
	}

	void Scoreboard::UnregisterPlayer(std::size_t playerIndex)
	{
		if (playerIndex >= m_players.size() || !m_players[playerIndex].has_value())
			return;

		auto& playerData = m_players[playerIndex].value();

		// Destroy widgets
		playerData.background->Destroy();
		for (auto& columnData : playerData.values)
			columnData.label->Destroy();

		m_players[playerIndex].reset();

		Layout();
	}

	void Scoreboard::UpdatePlayerTeam(std::size_t playerIndex, std::size_t teamId)
	{
		if (playerIndex >= m_players.size() || !m_players[playerIndex].has_value())
			return;

		auto& playerData = m_players[playerIndex];
		if (playerData->teamId != teamId)
		{
			playerData->teamId = teamId;

			Nz::Color teamColor = Nz::Color::White();
			if (teamId < m_teams.size())
				teamColor = m_teams[teamId].color;

			std::shared_ptr<Nz::Font> scoreMenuFont = Nz::Font::GetDefault();
			//Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");

			for (auto& playerColumn : playerData->values)
			{
				playerColumn.label->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, playerColumn.value, 18, 0, teamColor));
				playerColumn.label->Resize(playerColumn.label->GetPreferredSize());
			}

			Layout();
		}
	}

	void Scoreboard::UpdatePlayerValue(std::size_t playerIndex, std::size_t valueIndex, std::string value)
	{
		if (playerIndex >= m_players.size() || !m_players[playerIndex].has_value())
			return;

		auto& playerData = *m_players[playerIndex];
		if (valueIndex >= playerData.values.size())
			return;

		Nz::Color playerColor = Nz::Color::White();
		if (playerData.color)
			playerColor = *playerData.color;
		else if (playerData.teamId < m_teams.size())
			playerColor = m_teams[playerData.teamId].color;

		std::shared_ptr<Nz::Font> scoreMenuFont = Nz::Font::GetDefault();
		//Nz::FontRef scoreMenuFont = Nz::FontLibrary::Get("BW_ScoreMenu");

		auto& columnData = playerData.values[valueIndex];
		columnData.value = std::move(value);
		columnData.label->UpdateText(Nz::SimpleTextDrawer::Draw(scoreMenuFont, columnData.value, 18, 0, playerColor));
		columnData.label->Resize(columnData.label->GetPreferredSize());

		Layout();
	}

	void Scoreboard::Layout()
	{
		Nz::Vector2f size = GetSize();
		m_backgroundWidget->Resize(size);

		if (m_columns.empty())
			return;

		constexpr float padding = 5.f;

		float scoreboardWidth = size.x - padding * 2.f;

		float cursor = padding;
		float height = 0.f;
		float spaceBetweenColumn = scoreboardWidth / m_columns.size();

		Nz::StackArray<float> columnOffsets = NazaraStackArrayNoInit(float, m_columns.size());
		for (std::size_t i = 0; i < m_columns.size(); ++i)
		{
			auto& columnData = m_columns[i];
			columnOffsets[i] = cursor;

			columnData.widget->SetPosition({ cursor, padding });
			cursor += spaceBetweenColumn;

			height = std::max(height, columnData.widget->GetHeight());
		}

		float titleHeight = height;

		m_columnBackgroundWidget->Resize({ size.x, titleHeight });

		constexpr float playerMargin = 6.f;

		cursor = 0.f;
		auto HandlePlayer = [&](PlayerData& playerData)
		{
			height = 0.f;
			for (std::size_t columnIndex = 0; columnIndex < playerData.values.size(); ++columnIndex)
			{
				auto& columnData = playerData.values[columnIndex];
				float offset = columnOffsets[columnIndex];
				columnData.label->SetPosition({ offset, cursor });

				height = std::max(height, columnData.label->GetHeight());
			}

			playerData.background->Resize({ 0.f, height });
			playerData.background->SetPosition({ 0.f, cursor });

			cursor += height + playerMargin;
		};

		for (auto& playerOpt : m_players)
		{
			if (!playerOpt)
				continue;

			if (playerOpt->teamId < m_teams.size())
				continue;

			HandlePlayer(playerOpt.value());
		}

		for (std::size_t teamIndex = 0; teamIndex < m_teams.size(); ++teamIndex)
		{
			auto& teamData = m_teams[teamIndex];

			teamData.widget->SetPosition({ 0.f, cursor });
			cursor += teamData.widget->GetHeight();

			teamData.line->SetPosition({ 0.f, cursor });
			teamData.line->Resize({ size.x, 2.f });
			cursor += teamData.line->GetHeight() + 5.f;

			for (auto& playerOpt : m_players)
			{
				if (!playerOpt || playerOpt->teamId != teamIndex)
					continue;

				HandlePlayer(playerOpt.value());
			}
		}

		m_contentWidget->Resize({ scoreboardWidth, cursor + playerMargin });
		m_scrollArea->Resize({ scoreboardWidth, size.y - titleHeight - padding * 2.f }); // force layout update
		m_scrollArea->SetPosition({ padding, titleHeight + padding });

		float playerWidth = m_contentWidget->GetWidth();
		for (auto& playerOpt : m_players)
		{
			if (!playerOpt)
				continue;

			auto& playerData = playerOpt.value();
			playerData.background->Resize({ playerWidth, playerData.background->GetHeight() });
		}
	}
}
