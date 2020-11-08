RegisterClientScript()

function debug.DrawRect(layerIndex, rect, lifetime)
	match.CreateEntity({
		Type = "entity_debug_rect",
		LayerIndex = layerIndex,
		Position = rect:GetPosition(),
		Properties = {
			lifetime = lifetime or 0,
			rect = rect
		}
	})
end
