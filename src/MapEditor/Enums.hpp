// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_ENUMS_HPP
#define BURGWAR_MAPEDITOR_ENUMS_HPP

#include <NazaraUtils/Flags.hpp>

namespace bw
{
	enum class EntityInfoUpdate
	{
		EntityClass,
		EntityName,
		PositionRotation,
		Properties,

		Max = Properties
	};
}

namespace Nz
{
	template<>
	struct EnumAsFlags<bw::EntityInfoUpdate>
	{
		static constexpr bw::EntityInfoUpdate max = bw::EntityInfoUpdate::Max;
	};
}

namespace bw
{
	using EntityInfoUpdateFlags = Nz::Flags<bw::EntityInfoUpdate>;
}

#endif
