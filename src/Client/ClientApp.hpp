// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTAPP_HPP
#define BURGWAR_CLIENTAPP_HPP

#include <CoreLib/BurgApp.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/NetworkReactorManager.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Application.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>
#include <vector>

namespace bw
{
	class LocalMatch;
	class Match;
	class NetworkReactor;
	class NetworkSessionBridge;

	class ClientApp : public Ndk::Application, public BurgApp
	{
		public:
			ClientApp(int argc, char* argv[]);
			~ClientApp();

			int Run();

		private:
			std::shared_ptr<LocalMatch> CreateLocalMatch(ClientSession& session, const Packets::MatchData& matchData);

			std::vector<std::shared_ptr<LocalMatch>> m_localMatches;
			std::unique_ptr<Match> m_match;
			std::unique_ptr<ClientSession> m_clientSession;
			Nz::RenderWindow& m_mainWindow;
			LocalCommandStore m_commandStore;
			NetworkReactorManager m_networkReactors;
#if 0
			Ndk::EntityHandle m_camera;
			Ndk::EntityHandle m_burger;
			Ndk::EntityHandle m_weapon;
			Ndk::World& m_world;
			Nz::DegreeAnglef m_attackOriginAngle;
			Nz::SpriteRef m_burgerSprite;
			Nz::SpriteRef m_weaponSprite;
			bool m_isAttacking;
			bool m_isTargetingRight;
			bool m_isFacingRight;
			bool m_isOnGround;
			bool m_isMoving;
			float m_attackTimer;
#endif
	};
}

#include <Client/ClientApp.inl>

#endif