// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALMATCH_HPP
#define BURGWAR_CLIENT_LOCALMATCH_HPP

#include <Client/LocalInputController.hpp>
#include <Shared/EntityStore.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <hopstotch/hopscotch_map.h>
#include <Nazara/Platform/EventHandler.hpp>
#include <vector>

namespace bw
{
	class BurgApp;

	class LocalMatch
	{
		friend class ClientSession;

		public:
			LocalMatch(BurgApp& burgApp, ClientSession& session, const Packets::MatchData& matchData);
			~LocalMatch() = default;

			void Update(float elapsedTime);

		private:
			const Ndk::EntityHandle& CreateEntity(Nz::UInt32 serverId, const std::string& entityClassName, const Nz::Vector2f& createPosition, bool hasPlayerMovement);
			void DeleteEntity(Nz::UInt32 serverId);
			void MoveEntity(Nz::UInt32 serverId, const Nz::Vector2f& newPos, const Nz::Vector2f& newLinearVel, Nz::RadianAnglef newRot, Nz::RadianAnglef newAngularVel, bool isAirControlling, bool isFacingRight);
			void SendInputs();

			struct InputController
			{
				InputController(Nz::UInt8 playerIndex) :
				controller(playerIndex)
				{
				}

				LocalInputController controller;
				InputData lastInputData;
			};

			struct ServerEntity
			{
				Ndk::EntityOwner entity;
				Nz::Vector2f positionError = Nz::Vector2f::Zero();
				Nz::RadianAnglef rotationError = 0.f;
			};

			NazaraSlot(Nz::EventHandler, OnKeyPressed, m_onKeyPressedSlot);
			NazaraSlot(Nz::EventHandler, OnKeyReleased, m_onKeyReleasedSlot);

			std::vector<InputController> m_inputControllers;
			tsl::hopscotch_map<Nz::UInt32 /*serverEntityId*/, ServerEntity /*clientEntity*/> m_serverEntityIdToClient;
			Ndk::World m_world;
			Nz::LuaInstance m_luaInstance;
			BurgApp& m_application;
			ClientSession& m_session;
			EntityStore m_entityStore;
			Packets::PlayersInput m_inputPacket;
			float m_errorCorrectionTimer;
			float m_playerEntitiesTimer;
			float m_playerInputTimer;
	};
}

#include <Client/LocalMatch.inl>

#endif