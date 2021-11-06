RegisterClientScript()

local function ComputeBounds(layerIndex, padding)
	local entities = match.GetEntities(layerIndex)

	local bounds
	for _, entity in pairs(entities) do
		local entityBounds = entity:GetGlobalBounds()
		if bounds then
			bounds:ExtendToRect(entityBounds)
		else
			bounds = entityBounds
		end
	end

	bounds.x = bounds.x - padding
	bounds.y = bounds.y - padding
	bounds.width = bounds.width + padding * 2
	bounds.height = bounds.height + padding * 2

	return bounds
end

local entity = ScriptedEntity({
	Properties = {
		--{ Name = "auto_update", Type = PropertyType.Boolean, Default = true },
		{ Name = "padding", Type = PropertyType.Float, Default = 1000 },
		{ Name = "size", Type = PropertyType.FloatSize, Default = Vec2(0,0) }
	},
	EditorActions = {
		{
			Name = "recomputeArea",
			Label = "Recompute area",
			OnTrigger = function (entityEditor)
				local bounds = ComputeBounds(entityEditor:GetLayerIndex(), entityEditor:GetProperty("padding"))

				entityEditor:UpdatePosition(bounds:GetPosition())
				entityEditor:UpdateProperty("size", bounds:GetSize())
			end
		}
	}
})

entity:On("Init", function (self)
	local bounds
	--[[if self:GetProperty("auto_update") then
		bounds = ComputeBounds(self:GetLayerIndex(), self:GetProperty("padding"))
		self:SetPosition(bounds:GetPosition())
	else]]
		bounds = Rect(self:GetPosition(), self:GetProperty("size"))
	--end

	self:SetColliders({ 
		Collider = Rect(Vec2(), bounds:GetSize()),
		ColliderType = ColliderType.Callback,
		IsTrigger = true
	})

	if (EDITOR) then
		local color = { r = 50, g = 200, b = 50, a = 180 }
		local lineWidth = 2

		-- Top
		self:AddSprite({
			Color = color,
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(bounds.width, lineWidth),
		})

		-- Right
		self:AddSprite({
			Color = color,
			Offset = Vec2(bounds.width, 0),
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(lineWidth, bounds.height),
		})

		-- Bottom
		self:AddSprite({
			Color = color,
			Offset = Vec2(0, bounds.height),
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(bounds.width, lineWidth),
		})

		-- Left
		self:AddSprite({
			Color = color,
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(lineWidth, bounds.height),
		})
	end
end)

entity:On("CollisionStop", function (self, other)
	other:Kill()
end)
