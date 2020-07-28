RegisterClientScript()
RegisterClientAssets("placeholder/healthpack.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {}
})

function entity:Initialize()
	local colliderSize = Vec2(400, 200) / 4 / 2
	self:SetCollider({ Collider = Rect(-colliderSize, colliderSize), IsTrigger = true })
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.25, 0.25),
			TexturePath = "placeholder/healthpack.png"
		})
	end
end

if (SERVER) then
	function entity:OnCollisionStart(other)
		if (other.Name == "burger") then
			if (not other:IsFullHealth()) then
				other:Heal(250)
				self:Kill()
				self.Parent:OnPowerupConsumed()
			end
		end

		return false
	end
end