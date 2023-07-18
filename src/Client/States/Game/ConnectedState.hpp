// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CONNECTEDSTATE_HPP
#define BURGWAR_STATES_GAME_CONNECTEDSTATE_HPP

#include <CoreLib/SessionBridge.hpp>
#include <CoreLib/Utility/AverageValues.hpp>
#include <Client/States/AbstractState.hpp>
#include <Nazara/Core/Clock.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>
#include <optional>
#include <variant>

namespace bw
{
	class ClientSession;

	class ConnectedState final : public AbstractState
	{
		public:
			ConnectedState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, std::shared_ptr<AbstractState> firstState);
			~ConnectedState() = default;

		private:
			enum InfoFlags
			{
				DataUsage     = 1 << 0,
				DownloadSpeed = 1 << 1,
				Ping          = 1 << 2,
				UploadSpeed   = 1 << 3
			};

			void Enter(Nz::StateMachine& fsm) override;
			void LayoutWidgets() override;
			void OnConnectionLost();
			void OnConnectionRetrieved();
			void PollSessionInfo();
			void RefreshFlags();
			void UpdateSessionInfo();
			bool Update(Nz::StateMachine& fsm, Nz::Time elapsedTime) override;

			std::optional<SessionBridge::SessionInfo> m_lastSessionInfo;
			std::shared_ptr<AbstractState> m_firstState;
			std::shared_ptr<ClientSession> m_clientSession;
			EntityOwner m_connectionLostEntity;
			Nz::LabelWidget* m_downloadSpeedLabel;
			Nz::LabelWidget* m_pingLabel;
			Nz::LabelWidget* m_uploadSpeedLabel;
			Nz::MillisecondClock m_lastSessionClock;
			std::shared_ptr<Nz::Sprite> m_connectionLostSprite;
			Nz::Time m_connectionLostCounter;
			Nz::Time m_queryInfoTimer;
			Nz::Time m_updateInfoTimer;
			Nz::UInt8 m_connectionInfoFlags;
			Nz::UInt32 m_queryId;
			AverageValues<double> m_downloadSpeed;
			AverageValues<double> m_uploadSpeed;
			bool m_connectionLost;
	};
}

#include <Client/States/Game/ConnectedState.inl>

#endif
