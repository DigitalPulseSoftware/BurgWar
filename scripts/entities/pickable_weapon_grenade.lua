RegisterClientScript()
RegisterClientAssets("placeholder/potato.png")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {}

function ENTITY:Initialize()
	self:SetCollider({ Collider = Circle(Vec2(0, 0), 128 * 0.3), IsTrigger = true })
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.1, 0.1),
			TexturePath = "placeholder/potato.png"
		})
	end
end

function ENTITY:OnCollisionStart(other)
	if (SERVER and other.Name == "burger") then
		local owner = other:GetOwner()
		if (not owner:HasWeapon("weapon_grenade")) then
			owner:GiveWeapon("weapon_grenade")
			self:Kill()

			if (self.Parent) then
				self.Parent:OnPowerupConsumed()
			end
		end
	end

	return false
end

