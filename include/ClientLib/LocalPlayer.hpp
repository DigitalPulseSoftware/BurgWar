// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALPLAYER_HPP
#define BURGWAR_CLIENTLIB_LOCALPLAYER_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <string>

namespace bw
{
	class LocalPlayer;

	using LocalPlayerHandle = Nz::ObjectHandle<LocalPlayer>;

	class LocalPlayer : public Nz::HandledObject<LocalPlayer>
	{
		public:
			inline LocalPlayer(Nz::UInt16 playerIndex, std::string name);
			~LocalPlayer() = default;

			inline const std::string& GetName() const;
			inline Nz::UInt16 GetPing() const;
			inline Nz::UInt16 GetPlayerIndex() const;

			inline void UpdatePing(Nz::UInt16 ping);

			static constexpr Nz::UInt16 InvalidPing = std::numeric_limits<Nz::UInt16>::max();

		private:
			std::string m_name;
			Nz::UInt16 m_ping;
			Nz::UInt16 m_playerIndex;
	};
}

#include <ClientLib/LocalPlayer.inl>

#endif