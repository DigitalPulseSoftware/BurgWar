// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientApp.hpp>

namespace bw
{
	inline std::size_t ClientApp::AddReactor(std::unique_ptr<NetworkReactor> reactor)
	{
		m_reactors.emplace_back(std::move(reactor));
		return m_reactors.size() - 1;
	}

	inline void ClientApp::ClearReactors()
	{
		m_reactors.clear();
	}

	inline const LocalCommandStore& ClientApp::GetCommandStore() const
	{
		return m_commandStore;
	}

	inline Nz::RenderWindow& ClientApp::GetMainWindow() const
	{
		return m_mainWindow;
	}

	inline const std::unique_ptr<NetworkReactor>& ClientApp::GetReactor(std::size_t reactorId)
	{
		assert(reactorId < m_reactors.size());
		return m_reactors[reactorId];
	}

	inline std::size_t ClientApp::GetReactorCount() const
	{
		return m_reactors.size();
	}

}