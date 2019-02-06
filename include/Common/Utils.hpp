// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Common" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_COMMON_UTILS_HPP
#define BURGWAR_COMMON_UTILS_HPP

#include <type_traits>

namespace bw
{
	template<typename T>
	struct AlwaysFalse : std::false_type {};
}

#endif
