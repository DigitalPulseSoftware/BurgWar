RegisterClientScript()

ENTITY.IsNetworked = true

ENTITY.Properties = {
	{ Name = "layer", Type = PropertyType.Layer, Default = NoLayer, Shared = true },
	{ Name = "parallaxFactor", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true },
	{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "scale", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true },
}

if (CLIENT) then
	function ENTITY:Initialize()
		print("Should draw layer ", self:GetProperty("layer"))
		self:AddLayer({
			LayerIndex = self:GetProperty("layer"),
			RenderOrder = self:GetProperty("renderOrder"),
			Scale = self:GetProperty("scale")
		})
	end
end

function ENTITY:OnPlayerEnterLayer(player)
	print("Making layer visible")
	player:UpdateLayerVisibility(self:GetProperty("layer"), true)
end

function ENTITY:OnPlayerLeaveLayer(player)
	print("Making layer invisible")
	player:UpdateLayerVisibility(self:GetProperty("layer"), false)
end
