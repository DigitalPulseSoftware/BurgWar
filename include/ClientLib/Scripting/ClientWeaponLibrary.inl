// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <cassert>

namespace bw
{
	inline ClientWeaponLibrary::ClientWeaponLibrary(AssetStore& assetStore) :
	m_assetStore(assetStore)
	{
	}
}
