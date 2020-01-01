RegisterClientScript()
RegisterClientAssets("placeholder/healthpack.png")

ENTITY.IsNetworked = true

ENTITY.Properties = {}

function ENTITY:Initialize()
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
	function ENTITY:OnCollisionStart(other)
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