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

function debug.DrawText(layerIndex, text, lifetime)
	match.CreateEntity({
		Type = "entity_debug_text",
		LayerIndex = layerIndex,
		Position = text:GetPosition(),
		Properties= {
			lifetime = lifetime or 0,
			text = text
		}
	})
end
