// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <cassert>

namespace bw
{
	inline ClientWeaponLibrary::ClientWeaponLibrary(const Logger& logger, ClientAssetStore& assetStore) :
	SharedWeaponLibrary(logger),
	m_assetStore(assetStore)
	{
	}
}
