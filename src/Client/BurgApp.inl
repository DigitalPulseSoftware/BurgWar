// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/BurgApp.hpp>

namespace bw
{
	inline Nz::UInt64 bw::BurgApp::GetAppTime() const
	{
		return m_appTime;
	}

	inline const ServerCommandStore& BurgApp::GetCommandStore() const
	{
		return m_commandStore;
	}

	inline Nz::RenderWindow& BurgApp::GetMainWindow() const
	{
		return m_mainWindow;
	}

	inline std::size_t BurgApp::AddReactor(std::unique_ptr<NetworkReactor> reactor)
	{
		m_reactors.emplace_back(std::move(reactor));
		return m_reactors.size() - 1;
	}

	inline void BurgApp::ClearReactors()
	{
		m_reactors.clear();
	}

	inline const std::unique_ptr<NetworkReactor>& BurgApp::GetReactor(std::size_t reactorId)
	{
		assert(reactorId < m_reactors.size());
		return m_reactors[reactorId];
	}

	inline std::size_t BurgApp::GetReactorCount() const
	{
		return m_reactors.size();
	}

}