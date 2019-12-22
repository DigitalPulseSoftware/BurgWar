RegisterClientScript()
RegisterClientAssets("placeholder/physgun.png")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 0

ENTITY.Properties = {}

function ENTITY:Initialize()
	self:SetCollider(Circle(Vec2(0, 0) * 0.15, 128 * 0.3), true)
	self:EnableCollisionCallbacks(true)

	if (CLIENT) then
		self:AddSprite({
			TexturePath = "placeholder/physgun.png"
		})
	end
end

function ENTITY:OnCollisionStart(other)
	if (SERVER and other.Name == "burger") then
		local owner = other:GetOwner()
		if (not owner:HasWeapon("weapon_physics_bun")) then
			owner:GiveWeapon("weapon_physics_bun")
			self:Kill()
			self.Parent:OnPowerupConsumed()
		end
	end

	return false
end

