RegisterClientScript()

entity.IsNetworked = true

entity.Properties = {
	{ Name = "layer", Type = PropertyType.Layer, Default = NoLayer, Shared = true },
	{ Name = "parallaxFactor", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true },
	{ Name = "recursive", Type = PropertyType.Boolean, Default = false, Shared = true },
	{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true },
	{ Name = "scale", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true },
}

function entity:Initialize()
	-- FIXME
	if (EDITOR) then
		return
	end

	if (CLIENT) then
		self.isVisible = false
		if (engine_GetActiveLayer() == self:GetLayerIndex() or self:GetProperty("recursive")) then
			self:Show()
		end
	end
end

if (CLIENT) then
	function entity:Hide()
		assert(self.isVisible)
		self.isVisible = false

		self:ClearLayers()
	end

	function entity:Show()
		assert(not self.isVisible)
		self.isVisible = true

		self:AddLayer({
			LayerIndex = self:GetProperty("layer"),
			ParallaxFactor = self:GetProperty("parallaxFactor"),
			RenderOrder = self:GetProperty("renderOrder"),
			Scale = self:GetProperty("scale")
		})
	end
end

function entity:OnPlayerEnterLayer(player, newLayer)
	if (newLayer == self:GetLayerIndex() or self:GetProperty("recursive")) then
		local layer = self:GetProperty("layer")
		print("Making layer " .. layer .. " visible")
		player:UpdateLayerVisibility(layer, true)

		if (CLIENT) then

		end
	end
end

function entity:OnPlayerLeaveLayer(player, oldLayer)
	if (oldLayer == self:GetLayerIndex() or self:GetProperty("recursive")) then
		local layer = self:GetProperty("layer")
		print("Making layer " .. layer .. " invisible")
		player:UpdateLayerVisibility(layer, false)
	end
end

function entity:OnEnterLayer(newLayer)
	if (newLayer == self:GetLayerIndex() or self:GetProperty("recursive")) then
		local layer = self:GetProperty("layer")
		print("Making layer " .. layer .. " visible")
		self:Show()
	end
end

function entity:OnLeaveLayer(oldLayer)
	if (oldLayer == self:GetLayerIndex() or self:GetProperty("recursive")) then
		local layer = self:GetProperty("layer")
		print("Making layer " .. layer .. " invisible")
		self:Hide()
	end
end
