local entity = ScriptedEntity({})

entity:On("init", function (self)
	if (EDITOR) then
		self:AddSprite({
			Scale = Vec2(0.33, 0.33),
			TexturePath = "spawnpoint.png"
		})
	end
end)
