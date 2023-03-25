// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_PLAYWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_PLAYWINDOW_HPP

#include <CoreLib/Match.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <memory>
#include <optional>

namespace bw
{
	class ClientEditorAppComponent;
	class Map;
	class VirtualDirectory;

	class PlayWindow : public NazaraCanvas
	{
		public:
			PlayWindow(ClientEditorAppComponent& app, Map map, float tickRate, QWidget* parent = nullptr);
			~PlayWindow() = default;

		private:
			void OnUpdate(Nz::Time elapsedTime) override;

			void resizeEvent(QResizeEvent* event) override;

			entt::registry m_world;
			Nz::Canvas m_canvas;
			std::optional<ClientMatch> m_clientMatch;
			std::optional<Match> m_match;
			std::shared_ptr<ClientSession> m_session;
	};
}

#include <MapEditor/Widgets/PlayWindow.inl>

#endif
