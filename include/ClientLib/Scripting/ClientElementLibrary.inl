// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <cassert>

namespace bw
{
	inline ClientElementLibrary::ClientElementLibrary(const Logger& logger, AssetStore& assetStore) :
	SharedElementLibrary(logger),
	m_assetStore(assetStore)
	{
	}
}