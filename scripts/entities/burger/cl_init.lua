include("monautrefichier.lua")

print("coucou")

ENTITY.IsMoving = false
ENTITY.IsHoping = false

function ENTITY:OnInputUpdate(input)
	local isMoving = input.isMovingLeft or input.isMovingRight
	if (self.IsMoving ~= isMoving) then
		self.IsMoving = isMoving
		if (isMoving and not input.isJumping) then
			if (not self.IsHoping) then
				self.IsHoping = true
				while (self.IsMoving) do
					if (self:IsPlayerOnGround()) then
						animation.PositionByOffset(self.Entity, Vec2(0, 0), Vec2(0, -25), 0.15)
						animation.PositionByOffset(self.Entity, Vec2(0, -25), Vec2(0, 0), 0.15)
					else
						timer.Sleep(30)
					end
				end
				self.IsHoping = false
			end
		end
	end
end
