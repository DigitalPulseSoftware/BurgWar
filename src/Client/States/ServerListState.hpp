// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_SERVERLISTSTATE_HPP
#define BURGWAR_STATES_SERVERLISTSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <tsl/hopscotch_map.h>
#include <variant>
#include <vector>

namespace bw
{
	class ServerListState final : public AbstractState
	{
		public:
			ServerListState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~ServerListState() = default;

		private:
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();
			void OnDirectConnectionPressed();

			void RefreshServers(float elapsedTime);

			struct ServerData
			{
				tsl::hopscotch_map<std::string, float> masterServers;
			};

			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_directConnectButton;
			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ServerData> m_serverListData;
	};
}

#include <Client/States/JoinServerState.inl>

#endif
