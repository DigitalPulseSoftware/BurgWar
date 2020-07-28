RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true
})

if (EDITOR) then
	function entity:Initialize()
		self:AddSprite({
			Color = { r = 255, g = 255, b = 255, a = 120 },
			RenderOrder = 2000,
			Origin = Vec2(2, 2),
			Size = Vec2(4, 4)
		})
	end
end