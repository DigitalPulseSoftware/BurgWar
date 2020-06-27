RegisterClientScript()

WEAPON.Cooldown = 5
WEAPON.Scale = 0.25
WEAPON.Sprite = "placeholder/patator.png"
WEAPON.SpriteOrigin = Vec2(100, 160) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(20, -60) -- This should not be here
WEAPON.Animations = {}

local ammoSprite = "placeholder/potato.png"

RegisterClientAssets(WEAPON.Sprite)
RegisterClientAssets(ammoSprite)

if (SERVER) then
	function WEAPON:OnAttack()
		self.ChargeStart = match.GetSeconds()
	end

	function WEAPON:OnAttackFinish()
		local chargeFactor = math.clamp((match.GetSeconds() - self.ChargeStart) ^ 2, 0, 5) / 5

		local rotation = self:GetRotation() + 90
		if (not self:IsLookingRight()) then
			rotation = rotation + 180
		end

		local projectile = match.CreateEntity({
			Type = "entity_potato",
			LayerIndex = self:GetLayerIndex(),
			Owner = self:GetOwner(),
			Position = self:GetPosition() + self:GetDirection() * 360 * self.Scale,
			Rotation = rotation,
			Properties = {}
		})

		projectile:SetVelocity(self:GetDirection() * 1500 * chargeFactor)
	end
else
	WEAPON.ChargeBarFullsize = Vec2(60, 10)

	function WEAPON:Initialize()
		self.Potato = self:AddSprite({
			Offset = Vec2(360, -20) * self.Scale,
			Rotation = 90,
			RenderOrder = -2,
			TexturePath = ammoSprite,
			Scale = Vec2(0.1, 0.1)
		})

		self.ChargeBar = self:AddSprite({
			Offset = Vec2(50, -50),
			CornerColor = {
				TopLeft = { r = 255, g = 0, b = 0 },
				BottomLeft = { r = 255, g = 0, b = 0 }
			}
		})
		self.ChargeBar:Hide()
	end

	function WEAPON:OnAttack()
		self.ChargeStart = match.GetSeconds()
		self.IsCharging = true
		self.ChargeBar:SetSize(Vec2(0, self.ChargeBarFullsize.y))
		self.ChargeBar:Show()
	end

	function WEAPON:OnTick()
		if (self.IsCharging) then
			local chargeFactor = math.clamp((match.GetSeconds() - self.ChargeStart) ^ 2, 0, 5) / 5
			self.ChargeBar:SetSize(Vec2(self.ChargeBarFullsize.x * chargeFactor, self.ChargeBarFullsize.y))

			local rightColor = { r = 255 * (1 - chargeFactor), g = 255 * chargeFactor, b = 0 }
			self.ChargeBar:SetCornerColor("TopRight", rightColor)
			self.ChargeBar:SetCornerColor("BottomRight", rightColor)
		end
	end

	function WEAPON:OnAttackFinish()
		self.IsCharging = false
		self.ChargeBar:Hide()
		self.Potato:Hide()

		timer.Sleep(self.Cooldown * 1000)

		if (self.Potato:IsValid()) then
			self.Potato:Show()
		end
	end
end
