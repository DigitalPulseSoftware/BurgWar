// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientEditorAppComponent.hpp>

namespace bw
{
	inline ConfigFile& ClientEditorAppComponent::GetPlayerSettings()
	{
		return m_playerSettings;
	}

	inline const ConfigFile& ClientEditorAppComponent::GetPlayerSettings() const
	{
		return m_playerSettings;
	}
}
