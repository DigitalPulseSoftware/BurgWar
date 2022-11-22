// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectionState.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/MainMenuState.hpp>
#include <Client/States/Game/AuthenticationState.hpp>
#include <Client/States/Game/ConnectedState.hpp>
#include <Nazara/Network/Algorithm.hpp>

namespace bw
{
	ConnectionState::ConnectionState(std::shared_ptr<StateData> stateData, Address remoteAddress, std::shared_ptr<AbstractState> previousState) :
	ConnectionState(std::move(stateData), AddressList{ std::move(remoteAddress) }, std::move(previousState))
	{
	}

	ConnectionState::ConnectionState(std::shared_ptr<StateData> stateData, AddressList remoteAddresses, std::shared_ptr<AbstractState> previousState) :
	CancelableState(stateData, std::move(previousState)),
	m_currentAddressIndex(0),
	m_addresses(std::move(remoteAddresses)),
	m_timeBeforeGivingUp(0.f)
	{
		ClientApp* app = GetStateData().app;

		m_clientSession = std::make_shared<ClientSession>(*app);
		m_clientSessionConnectedSlot.Connect(m_clientSession->OnConnected, [this] (ClientSession*)
		{
			UpdateStatus("Connected, authenticating...", Nz::Color::White);

			auto authState = std::make_shared<AuthenticationState>(GetStateDataPtr(), m_clientSession, GetOriginalState());
			SwitchToState(std::make_shared<ConnectedState>(GetStateDataPtr(), m_clientSession, std::move(authState)), 0.5f);
		});

		m_clientSessionDisconnectedSlot.Connect(m_clientSession->OnDisconnected, [this](ClientSession*)
		{
			HandleConnectionFailure();
		});

		ProcessNextAddress();
	}

	ConnectionState::~ConnectionState()
	{
		if (m_resolvingData && m_resolvingData->thread.joinable())
			m_resolvingData->thread.join();
	}

	void ConnectionState::HandleConnectionFailure()
	{
		bwLog(GetStateData().app->GetLogger(), LogLevel::Error, "connection to address #{0}/{1} failed", m_currentAddressIndex + 1, m_addresses.size());

		m_currentAddressIndex++;
		ProcessNextAddress();
	}

	void ConnectionState::OnCancelled()
	{
		m_clientSessionDisconnectedSlot.Disconnect();
		m_clientSession->Disconnect();
	}

	void ConnectionState::ProcessNextAddress()
	{
		if (m_currentAddressIndex >= m_addresses.size())
		{
			bwLog(GetStateData().app->GetLogger(), LogLevel::Error, "no more addresses available, connection failed");

			UpdateStatus("Failed to connect to server", Nz::Color::Red);
			Cancel(3.f);
			return;
		}

		std::visit([this](auto&& value)
		{
			ProcessNextAddress(value);
		}, m_addresses[m_currentAddressIndex]);
	}

	void ConnectionState::ProcessNextAddress(const ServerName& name)
	{
		// Try to parse as IP
		Nz::IpAddress ipAddress;
		if (ipAddress.BuildFromAddress(name.hostname.c_str()))
		{
			ipAddress.SetPort(name.port);
			return ProcessNextAddress(ipAddress);
		}

		// Not a regular IP, try to resolve it
		auto& resolve = m_resolvingData.emplace();
		resolve.serverName = name;
		resolve.thread = std::thread([=]
		{
			bwLog(GetStateData().app->GetLogger(), LogLevel::Debug, "resolving {0}:{1}...", m_resolvingData->serverName.hostname, name.port);

			Nz::ResolveError resolveError;
			std::vector<Nz::HostnameInfo> serverAddresses = Nz::IpAddress::ResolveHostname(Nz::NetProtocol::Any, m_resolvingData->serverName.hostname, Nz::String::Number(name.port), &resolveError);
			if (serverAddresses.empty())
			{
				m_resolvingData->result = tl::unexpected<std::string>(Nz::ErrorToString(resolveError));
				bwLog(GetStateData().app->GetLogger(), LogLevel::Debug, "resolution of {0}:{1} failed: {2}", m_resolvingData->serverName.hostname, name.port, m_resolvingData->result.error());
			}
			else
			{
				std::vector<Nz::IpAddress> addresses;
				addresses.reserve(serverAddresses.size());
				for (const auto& hostnameInfo : serverAddresses)
					addresses.push_back(hostnameInfo.address);
			
				bwLog(GetStateData().app->GetLogger(), LogLevel::Debug, "resolution of {0}:{1} succeeded ({2} address(es) found)", m_resolvingData->serverName.hostname, name.port, addresses.size());
				m_resolvingData->result = std::move(addresses);
			}

			m_resolvingData->hasResult = true;
		});

		UpdateStatus("Resolving " + name.hostname + "...", Nz::Color::White);
	}

	void ConnectionState::ProcessNextAddress(const Nz::IpAddress& address)
	{
		ClientApp* app = GetStateData().app;
		auto& networkManager = app->GetReactorManager();

		bwLog(GetStateData().app->GetLogger(), LogLevel::Debug, "connecting to {0}...", address.ToString());

		auto sessionBridge = networkManager.ConnectToServer(address, 0);
		if (sessionBridge)
		{
			m_clientSession->Connect(std::move(sessionBridge));
			m_timeBeforeGivingUp = 10.f;

			UpdateStatus("Connecting to " + address.ToString() + "...", Nz::Color::White);
		}
		else
			HandleConnectionFailure();
	}

	void ConnectionState::ProcessNextAddress(LocalSessionManager* sessionManager)
	{
		bwLog(GetStateData().app->GetLogger(), LogLevel::Debug, "connecting using local session...");
		m_clientSession->Connect(sessionManager->CreateSession());
		m_timeBeforeGivingUp = 3.f; //< Should be instant
	}
	
	bool ConnectionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!CancelableState::Update(fsm, elapsedTime))
			return false;

		if (m_resolvingData)
		{
			if (m_resolvingData->hasResult)
			{
				m_resolvingData->thread.join();

				if (m_resolvingData->result.has_value())
				{
					// Register resolved addresses as next addresses
					const auto& addresses = m_resolvingData->result.value();
					for (auto resultIt = addresses.rbegin(); resultIt != addresses.rend(); ++resultIt)
						m_addresses.emplace(m_addresses.begin() + m_currentAddressIndex, *resultIt);
				}
				else
					bwLog(GetStateData().app->GetLogger(), LogLevel::Error, "hostname resolution of {0} failed: {1}", m_resolvingData->serverName.hostname, m_resolvingData->result.error());

				m_resolvingData.reset();
				ProcessNextAddress();
			}
		}
		else if (m_timeBeforeGivingUp > 0.f)
		{
			if ((m_timeBeforeGivingUp -= elapsedTime) < 0.f)
				m_clientSession->Disconnect();
		}

		return true;
	}
}
