// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_BURGAPP_HPP
#define BURGWAR_CLIENT_BURGAPP_HPP

#include <Shared/NetworkReactor.hpp>
#include <Shared/MatchSessions.hpp>
#include <Client/ServerCommandStore.hpp>
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
	class Match;
	class NetworkClientBridge;
	class NetworkReactor;

	class BurgApp : public Ndk::Application
	{
		friend class ClientSession;

		public:
			BurgApp(int argc, char* argv[]);
			~BurgApp();

			inline std::size_t AddReactor(std::unique_ptr<NetworkReactor> reactor);
			inline void ClearReactors();

			inline Nz::UInt64 GetAppTime() const;
			inline const ServerCommandStore& GetCommandStore() const;
			inline Nz::RenderWindow& GetMainWindow() const;
			inline const std::unique_ptr<NetworkReactor>& GetReactor(std::size_t reactorId);
			inline std::size_t GetReactorCount() const;

			int Run();

		private:
			std::shared_ptr<NetworkClientBridge> ConnectNewServer(const Nz::IpAddress& serverAddress, Nz::UInt32 data);

			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data);
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data);
			void HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo);
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet);

			std::vector<std::unique_ptr<NetworkReactor>> m_reactors;
			std::vector<std::shared_ptr<NetworkClientBridge>> m_connections;
			ServerCommandStore m_commandStore;
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