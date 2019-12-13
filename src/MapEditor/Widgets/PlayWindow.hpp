// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_PLAYWINDOW_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_PLAYWINDOW_HPP

#include <CoreLib/Match.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <memory>
#include <optional>

namespace bw
{
	class BurgApp;
	class Map;
	class VirtualDirectory;

	class PlayWindow : public NazaraCanvas
	{
		public:
			PlayWindow(BurgApp& app, Map map, std::shared_ptr<VirtualDirectory> assetFolder, std::shared_ptr<VirtualDirectory> scriptFolder, float tickRate, QWidget* parent = nullptr);
			~PlayWindow() = default;

		private:
			void OnUpdate(float elapsedTime) override;

			Ndk::World m_world;
			Ndk::Canvas m_canvas;
			std::optional<LocalMatch> m_localMatch;
			Match m_match;
			std::shared_ptr<ClientSession> m_session;
			QTimer m_updateTimer;
	};
}

#include <MapEditor/Widgets/PlayWindow.inl>

#endif