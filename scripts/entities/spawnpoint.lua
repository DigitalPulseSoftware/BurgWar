ENTITY.IsNetworked = false

function ENTITY:Initialize()
	if (EDITOR) then
		self:AddSprite("spawnpoint.png", Vec2(0.33, 0.33))
	end
end
