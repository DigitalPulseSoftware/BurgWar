// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_LAYERVISIBILITYDIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_LAYERVISIBILITYDIALOG_HPP

#include <CoreLib/LayerIndex.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QDialog>
#include <tsl/hopscotch_map.h>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;

namespace bw
{
	struct LayerVisibilityInfo
	{
		LayerIndex layerIndex;
		Nz::DegreeAnglef rotation = Nz::DegreeAnglef::Zero();
		Nz::Int32 renderOrder = -1;
		Nz::Vector2f offset = Nz::Vector2f::Zero();
		Nz::Vector2f parallaxFactor = Nz::Vector2f::Unit();
		Nz::Vector2f scale = Nz::Vector2f::Unit();
	};

	class Float2SpinBox;
	class Map;

	class LayerVisibilityDialog : public QDialog
	{
		public:
			LayerVisibilityDialog(LayerIndex excludedLayerIndex, const Map& map, QWidget* parent = nullptr);
			LayerVisibilityDialog(LayerIndex excludedLayerIndex, LayerVisibilityInfo layerInfo, const Map& map, QWidget* parent = nullptr);
			~LayerVisibilityDialog() = default;

			LayerVisibilityInfo GetLayerVisibilityInfo() const;

		private:
			void OnAccept();

			QComboBox* m_layer;
			QDoubleSpinBox* m_rotation;
			Float2SpinBox* m_offset;
			Float2SpinBox* m_parallaxFactor;
			Float2SpinBox* m_scale;
			QSpinBox* m_renderOrder;
	};
}

#include <MapEditor/Widgets/LayerVisibilityDialog.inl>

#endif