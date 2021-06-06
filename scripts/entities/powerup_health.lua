RegisterClientScript()
RegisterClientAssets("placeholder/healthpack.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {}
})

entity.Passthrough = true

entity:On("Init", function (self)
	local colliderSize = Vec2(400, 200) / 4 / 2
	self:SetColliders({
		Collider = Rect(-colliderSize, colliderSize),
		IsTrigger = true,
		ColliderType = ColliderType.Callback
	})

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.25, 0.25),
			TexturePath = "placeholder/healthpack.png"
		})
	end
end)

if (SERVER) then
	entity:On("CollisionStart", function (self, other)
		if (other.IsPlayerEntity) then
			if (not other:IsFullHealth()) then
				other:Heal(250)
				self:Kill()
			end
		end

		return false
	end)
end
