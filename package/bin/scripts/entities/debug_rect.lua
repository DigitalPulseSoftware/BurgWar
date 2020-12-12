RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "rect", Type = PropertyType.FloatRect, Shared = true },
		{ Name = "lifetime", Type = PropertyType.Float, Shared = true }
	}
})

if (not EDITOR) then
	entity:On("init", function (self)
		local lifetime = self:GetProperty("lifetime")
		if (lifetime > 0) then
			self:SetLifeTime(lifetime)
		end
	end)
end

if (CLIENT) then
	entity:On("init", function (self)
		local rect = self:GetProperty("rect")

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
	end)
end
