// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTPLAYER_HPP
#define BURGWAR_CLIENTLIB_CLIENTPLAYER_HPP

#include <CoreLib/EntityId.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <optional>
#include <string>

namespace bw
{
	class ClientPlayer;

	using ClientPlayerHandle = Nz::ObjectHandle<ClientPlayer>;

	class BURGWAR_CLIENTLIB_API ClientPlayer : public Nz::HandledObject<ClientPlayer>
	{
		public:
			inline ClientPlayer(Nz::UInt16 playerIndex, std::string name, std::optional<Nz::UInt8> localPlayerIndex);
			ClientPlayer(const ClientPlayer&) = delete;
			ClientPlayer(ClientPlayer&&) noexcept = default;
			~ClientPlayer() = default;

			inline EntityId GetControlledEntityId() const;
			inline Nz::UInt8 GetLocalPlayerIndex() const;
			inline const std::string& GetName() const;
			inline Nz::UInt16 GetPing() const;
			inline Nz::UInt16 GetPlayerIndex() const;

			inline bool IsLocalPlayer() const;

			inline const std::string& ToString() const;

			inline void UpdateControlledEntityId(EntityId controlledEntityId);
			inline void UpdateName(std::string name);
			inline void UpdatePing(Nz::UInt16 ping);

			ClientPlayer& operator=(const ClientPlayer&) = delete;
			ClientPlayer& operator=(ClientPlayer&&) noexcept = default;

			static constexpr Nz::UInt16 InvalidPing = std::numeric_limits<Nz::UInt16>::max();

		private:
			std::optional<Nz::UInt8> m_localPlayerIndex;
			std::string m_name;
			Nz::UInt16 m_ping;
			Nz::UInt16 m_playerIndex;
			EntityId m_controlledEntityId;
	};
}

#include <ClientLib/ClientPlayer.inl>

#endif
