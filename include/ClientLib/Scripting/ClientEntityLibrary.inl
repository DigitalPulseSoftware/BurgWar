// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityLibrary.hpp>
#include <cassert>

namespace bw
{
	inline ClientEntityLibrary::ClientEntityLibrary(const Logger& logger, ClientAssetStore& assetStore) :
	SharedEntityLibrary(logger),
	m_assetStore(assetStore)
	{
	}
}
