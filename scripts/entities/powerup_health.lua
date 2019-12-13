RegisterClientScript()
RegisterClientAssets("placeholder/healthpack.png")

ENTITY.IsNetworked = true

ENTITY.Properties = {}

function ENTITY:Initialize()
	local colliderSize = Vec2(400, 200) / 4 / 2
	self:SetCollider(Rect(-colliderSize, colliderSize), true)
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.25, 0.25),
			TexturePath = "placeholder/healthpack.png"
		})
	end
end

function ENTITY:OnCollisionStart(other)
	if (SERVER and other.Name == "burger") then
		if (not other:IsFullHealth()) then
			other:Heal(250)
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end

