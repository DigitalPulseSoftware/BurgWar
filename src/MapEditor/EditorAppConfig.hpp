// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_EDITORAPPCONFIG_HPP
#define BURGWAR_EDITORAPPCONFIG_HPP

#include <CoreLib/SharedAppConfig.hpp>

namespace bw
{
	class EditorWindow;

	class EditorAppConfig : public SharedAppConfig
	{
		public:
			EditorAppConfig(EditorWindow& app);
			~EditorAppConfig() = default;
	};
}

#include <MapEditor/EditorAppConfig.inl>

#endif
