// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <cassert>

namespace bw
{
	inline EditorEntityStore::EditorEntityStore(MapCanvas& mapCanvas, ClientAssetStore& assetStore, const Logger& logger, std::shared_ptr<ScriptingContext> context) :
	ClientEditorEntityStore(assetStore, logger, context),
	m_mapCanvas(mapCanvas)
	{
	}
}
