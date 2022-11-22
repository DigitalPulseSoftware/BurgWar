// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCOREBOARD_HPP
#define BURGWAR_CLIENTLIB_SCOREBOARD_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>
#include <Nazara/Widgets/ImageWidget.hpp>
#include <Nazara/Widgets/ScrollAreaWidget.hpp>
#include <optional>

namespace bw
{
	class Logger;
	class Scoreboard;

	using ScoreboardHandle = Nz::ObjectHandle<Scoreboard>;

	class BURGWAR_CLIENTLIB_API Scoreboard : public Nz::BaseWidget, public Nz::HandledObject<Scoreboard>
	{
		public:
			Scoreboard(Nz::BaseWidget* parent, const Logger& logger);
			Scoreboard(const Scoreboard&) = delete;
			Scoreboard(Scoreboard&&) = delete;
			~Scoreboard();

			std::size_t AddColumn(std::string name);
			std::size_t AddTeam(std::string name, Nz::Color color);

			void RegisterPlayer(std::size_t playerIndex, std::size_t teamId, std::vector<std::string> values, std::optional<Nz::Color> color = {}, bool isLocalPlayer = false);

			std::string ToString() const;

			void UnregisterPlayer(std::size_t playerIndex);

			void UpdatePlayerTeam(std::size_t playerIndex, std::size_t teamId);
			void UpdatePlayerValue(std::size_t playerIndex, std::size_t valueIndex, std::string value);

			Scoreboard& operator=(const Scoreboard&) = delete;
			Scoreboard& operator=(Scoreboard&&) = delete;

			static constexpr std::size_t InvalidTeam = std::numeric_limits<std::size_t>::max();

		private:
			void Layout() override;

			struct PlayerData
			{
				struct ColumnData
				{
					std::string value;
					Nz::LabelWidget* label;
				};

				Nz::BaseWidget* background;
				std::optional<Nz::Color> color;
				std::size_t teamId;
				std::vector<ColumnData> values;
			};

			struct ColumnData
			{
				std::string name;
				Nz::LabelWidget* widget;
			};

			struct TeamData
			{
				std::string name;
				Nz::Color color;
				Nz::BaseWidget* background;
				Nz::ImageWidget* line;
				Nz::LabelWidget* widget;
			};

			std::vector<ColumnData> m_columns;
			std::vector<std::optional<PlayerData>> m_players;
			std::vector<TeamData> m_teams;
			Nz::BaseWidget* m_backgroundWidget;
			Nz::BaseWidget* m_columnBackgroundWidget;
			Nz::BaseWidget* m_contentWidget;
			Nz::ScrollAreaWidget* m_scrollArea;
			const Logger& m_logger;
	};
}

#include <ClientLib/Scoreboard.inl>

#endif
