// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_SERVERLISTSTATE_HPP
#define BURGWAR_STATES_SERVERLISTSTATE_HPP

#include <Nazara/Network/WebService.hpp>
#include <Client/States/AbstractState.hpp>
#include <Nazara/Widgets.hpp>
#include <nlohmann/json_fwd.hpp>
#include <tsl/hopscotch_map.h>
#include <optional>
#include <variant>
#include <vector>

namespace bw
{
	class ServerListState final : public AbstractState
	{
		public:
			ServerListState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState);
			~ServerListState();

		private:
			void Enter(Nz::StateMachine& fsm) override;
			void Leave(Nz::StateMachine& fsm) override;
			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

			void LayoutWidgets() override;

			void OnBackPressed();
			void OnDirectConnectionPressed();
			void OnServerConnectionPressed(const std::string& masterServer, const std::string& uuid);

			void RefreshServers(Nz::Time elapsedTime);
			void UpdateServerList(const std::string& masterServer, const nlohmann::json& serverListDoc);

			struct ServerData
			{
				Nz::ButtonWidget* connectButton;
				Nz::LabelWidget* infoLabel;
				std::string serverName;
			};

			struct MasterServerData
			{
				tsl::hopscotch_map<std::string, ServerData> serverList;
				Nz::Time timeBeforeRefresh = Nz::Time::Zero();
				bool receivedData = false; //< Did we already successfully refresh from this master server?
			};

			Nz::BaseWidget* m_serverListWidget;
			Nz::ButtonWidget* m_backButton;
			Nz::ButtonWidget* m_directConnectButton;
			Nz::ScrollAreaWidget* m_serverListScrollbar;
			Nz::WebService* m_webService;
			std::shared_ptr<AbstractState> m_previousState;
			std::shared_ptr<AbstractState> m_nextGameState;
			std::shared_ptr<AbstractState> m_nextState;
			std::vector<std::reference_wrapper<const ServerData>> m_tempOrderedServerList;
			tsl::hopscotch_map<std::string, MasterServerData> m_masterServers;
	};
}

#include <Client/States/JoinServerState.inl>

#endif
