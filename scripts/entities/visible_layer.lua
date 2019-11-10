RegisterClientScript()

ENTITY.IsNetworked = true

ENTITY.Properties = {
	{ Name = "layer", Type = PropertyType.Layer, Default = NoLayer, Shared = true },
	{ Name = "parallax_factor", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true },
	{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "scale", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true },
}

function ENTITY:Initialize()
	print("Should draw layer ", self:GetProperty("layer"))
end

function ENTITY:OnPlayerEnterLayer(player)
	player:UpdateLayerVisibility(self:GetProperty("layer"), true)
end

function ENTITY:OnPlayerLeaveLayer(player)
	player:UpdateLayerVisibility(self:GetProperty("layer"), false)
end
