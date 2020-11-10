RegisterClientScript()

local entity = ScriptedEntity({
	Base = "entity_tilemap",
	IsNetworked = true
})

if (CLIENT) then
	entity.Alpha = 255
	entity.IsVisible = true

	entity:On("tick", function (self)
		local origin = self:GetPosition()
		local rect = Rect(origin, origin + self.Tilemap:GetSize())
		
		self.IsVisible = not rect:Contains(engine_GetPlayerPosition(0))
	end)

	entity:On("frame", function (self)
		if (self.IsVisible and self.Alpha ~= 255) then
			self:UpdateAlpha(self.Alpha + 200 * render.GetFrametime())
		elseif (not self.IsVisible and self.Alpha ~= 127) then
			self:UpdateAlpha(self.Alpha - 200 * render.GetFrametime())
		end
	end)

	function entity:UpdateAlpha(value)
		self.Alpha = math.floor(math.clamp(value, 0, 255))

		local color = { r = 255, g = 255, b = 255, a = self.Alpha }

		local mapSize = self.Tilemap:GetMapSize()
		for y = 0, mapSize.y - 1 do
			for x = 0, mapSize.x - 1 do
				self.Tilemap:SetTileColor(x, y, color)
			end
		end

	end
end
