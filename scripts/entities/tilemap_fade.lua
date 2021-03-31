RegisterClientScript()

local entity = ScriptedEntity({
	Base = "entity_tilemap",
	IsNetworked = true
})

if (CLIENT) then
	local maxAlpha = 255
	local minAlpha = 120
	local alphaRate = 200

	entity.Alpha = 255
	entity.IsVisible = true

	entity:On("Tick", function (self)
		local origin = self:GetPosition()
		local rect = Rect(origin, origin + self.Tilemap:GetSize())

		local playerPos = engine_GetPlayerPosition(0)
		self.IsVisible = not playerPos or not rect:Contains(playerPos)
	end)

	entity:On("Frame", function (self)
		if (self.IsVisible and self.Alpha ~= maxAlpha) then
			self:UpdateAlpha(self.Alpha + alphaRate * render.GetFrametime())
		elseif (not self.IsVisible and self.Alpha ~= minAlpha) then
			self:UpdateAlpha(self.Alpha - alphaRate * render.GetFrametime())
		end
	end)

	function entity:UpdateAlpha(value)
		self.Alpha = math.floor(math.clamp(value, minAlpha, maxAlpha))

		local color = { r = 255, g = 255, b = 255, a = self.Alpha }

		local mapSize = self.Tilemap:GetMapSize()
		for y = 0, mapSize.y - 1 do
			for x = 0, mapSize.x - 1 do
				self.Tilemap:SetTileColor(x, y, color)
			end
		end
	end
end
