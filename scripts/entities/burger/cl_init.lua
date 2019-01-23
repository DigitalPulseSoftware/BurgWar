include("monautrefichier.lua")

print("coucou")

ENTITY.IsMoving = false
ENTITY.IsHoping = false

function ENTITY:OnInputUpdate(input)
	print("[client] Input changed")

	local isMoving = input.isMovingLeft or input.isMovingRight
	if (self.IsMoving ~= isMoving) then
		self.IsMoving = isMoving
		if (isMoving and not input.isJumping) then
			if (not self.IsHoping) then
				self.IsHoping = true
				while (self.IsMoving) do
					animation.PositionByOffset(self.Entity, Vec2(0, 0), Vec2(0, -25), 0.15)
					animation.PositionByOffset(self.Entity, Vec2(0, -25), Vec2(0, 0), 0.15)
				end
				self.IsHoping = false
			end
		else
			print("Stopped move")
		end
	end
end
