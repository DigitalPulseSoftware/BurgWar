// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALPLAYER_HPP
#define BURGWAR_CLIENTLIB_LOCALPLAYER_HPP

#include <CoreLib/EntityId.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <string>

namespace bw
{
	class LocalPlayer;

	using LocalPlayerHandle = Nz::ObjectHandle<LocalPlayer>;

	class BURGWAR_CLIENTLIB_API LocalPlayer : public Nz::HandledObject<LocalPlayer>
	{
		public:
			inline LocalPlayer(Nz::UInt16 playerIndex, std::string name);
			LocalPlayer(const LocalPlayer&) = delete;
			LocalPlayer(LocalPlayer&&) noexcept = default;
			~LocalPlayer() = default;

			inline EntityId GetControlledEntityId() const;
			inline const std::string& GetName() const;
			inline Nz::UInt16 GetPing() const;
			inline Nz::UInt16 GetPlayerIndex() const;

			inline const std::string& ToString() const;

			inline void UpdateControlledEntityId(EntityId controlledEntityId);
			inline void UpdateName(std::string name);
			inline void UpdatePing(Nz::UInt16 ping);

			LocalPlayer& operator=(const LocalPlayer&) = delete;
			LocalPlayer& operator=(LocalPlayer&&) noexcept = default;

			static constexpr Nz::UInt16 InvalidPing = std::numeric_limits<Nz::UInt16>::max();

		private:
			std::string m_name;
			Nz::UInt16 m_ping;
			Nz::UInt16 m_playerIndex;
			EntityId m_controlledEntityId;
	};
}

#include <ClientLib/LocalPlayer.inl>

#endif
