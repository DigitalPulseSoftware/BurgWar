// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Integer4SpinBox.hpp>

namespace bw
{
	inline Integer4SpinBox::Integer4SpinBox(const Nz::Vector4i64& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	Integer4SpinBox(labelMode, dir, parent)
	{
		setValue(value);
	}
}