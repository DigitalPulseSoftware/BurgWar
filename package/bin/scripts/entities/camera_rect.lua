RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "size", Type = PropertyType.FloatSize, Shared = true },
		{ Name = "transition_time", Type = PropertyType.FloatSize, Default = Vec2(1, 1), Shared = true }
	}
})


entity:On("init", function (self)
	if (EDITOR) then
		local rect = self:GetRect()

		local color = { r = 50, g = 200, b = 50, a = 180 }
		local lineWidth = 2

		-- Top
		self:AddSprite({
			Color = color,
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(rect.width, lineWidth),
		})

		-- Right
		self:AddSprite({
			Color = color,
			Offset = Vec2(rect.width, 0),
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(lineWidth, rect.height),
		})

		-- Bottom
		self:AddSprite({
			Color = color,
			Offset = Vec2(0, rect.height),
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(rect.width, lineWidth),
		})

		-- Left
		self:AddSprite({
			Color = color,
			Origin = Vec2(0,0),
			RenderOrder = 2000,
			Size = Vec2(lineWidth, rect.height),
		})
	end
end)

function entity:GetRect()
	local position = self:GetPosition()
	local size = self:GetProperty("size")

	return Rect(position, position + size)
end

function entity:GetTransitionTime()
	return self:GetProperty("transition_time")
end