// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCOREBOARD_HPP
#define BURGWAR_CLIENTLIB_SCOREBOARD_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <NDK/Widgets/ScrollAreaWidget.hpp>
#include <optional>

namespace bw
{
	class Logger;
	class Scoreboard;

	using ScoreboardHandle = Nz::ObjectHandle<Scoreboard>;

	class Scoreboard : public Ndk::BaseWidget, public Nz::HandledObject<Scoreboard>
	{
		public:
			Scoreboard(Ndk::BaseWidget* parent, const Logger& logger);
			Scoreboard(const Scoreboard&) = delete;
			Scoreboard(Scoreboard&&) = delete;
			~Scoreboard();

			std::size_t AddColumn(std::string name);
			std::size_t AddTeam(std::string name, Nz::Color color);

			void RegisterPlayer(std::size_t playerIndex, std::size_t teamId, std::vector<std::string> values, bool isLocalPlayer = false);

			Nz::String ToString() const;

			void UnregisterPlayer(std::size_t playerIndex);

			void UpdatePlayerTeam(std::size_t playerIndex, std::size_t teamId);
			void UpdatePlayerValue(std::size_t playerIndex, std::size_t valueIndex, std::string value);

			Scoreboard& operator=(const Scoreboard&) = delete;
			Scoreboard& operator=(Scoreboard&&) = delete;

		private:
			void Layout() override;

			struct PlayerData
			{
				struct ColumnData
				{
					std::string value;
					Ndk::LabelWidget* label;
				};

				Ndk::BaseWidget* background;
				std::size_t teamId;
				std::vector<ColumnData> values;
			};

			struct ColumnData
			{
				std::string name;
				Ndk::LabelWidget* widget;
			};

			struct TeamData
			{
				std::string name;
				Nz::Color color;
				Ndk::BaseWidget* background;
				Ndk::LabelWidget* widget;
			};

			std::vector<ColumnData> m_columns;
			std::vector<std::optional<PlayerData>> m_players;
			std::vector<TeamData> m_teams;
			Ndk::BaseWidget* m_backgroundWidget;
			Ndk::BaseWidget* m_columnBackgroundWidget;
			Ndk::BaseWidget* m_contentWidget;
			Ndk::ScrollAreaWidget* m_scrollArea;
			const Logger& m_logger;
	};
}

#include <ClientLib/Scoreboard.inl>

#endif
