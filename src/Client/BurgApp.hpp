// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_BURGAPP_HPP
#define BURGWAR_CLIENT_BURGAPP_HPP

#include <Shared/NetworkReactor.hpp>
#include <Client/Player.hpp>
#include <Client/ServerCommandStore.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Application.hpp>
#include <NDK/Entity.hpp>
#include <NDK/World.hpp>
#include <vector>

namespace bw
{
	class Match;
	class NetworkReactor;
	class ServerConnection;

	class BurgApp : public Ndk::Application
	{
		friend ServerConnection;

		public:
			BurgApp(int argc, char* argv[]);
			~BurgApp();

			inline Nz::UInt64 GetAppTime() const;
			inline const ServerCommandStore& GetCommandStore() const;
			inline Nz::RenderWindow& GetMainWindow() const;

			int Run();

		private:
			inline std::size_t AddReactor(std::unique_ptr<NetworkReactor> reactor);
			inline void ClearReactors();
			inline const std::unique_ptr<NetworkReactor>& GetReactor(std::size_t reactorId);
			inline std::size_t GetReactorCount() const;

			bool ConnectNewServer(const Nz::String& serverHostname, Nz::UInt32 data, ServerConnection* connection, std::size_t* peerId, NetworkReactor** peerReactor);

			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data);
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data);
			void HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo);
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet);

			std::vector<std::unique_ptr<NetworkReactor>> m_reactors;
			std::vector<ServerConnection*> m_servers;
			ServerCommandStore m_commandStore;
			Player m_testPlayer;
			std::unique_ptr<Match> m_match;
			Nz::RenderWindow& m_mainWindow;
			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
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

#include <Client/BurgApp.inl>

#endif