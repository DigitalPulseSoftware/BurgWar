function GM:OnPlayerDeath(player, attacker)
	print("Ah ben tiens c'est dommage")
end

function GM:OnTick()
end

function GM:OnInit()
	print(self, "Le match a été créé")

	for i = 0, 10 do
		self:CreateEntity("entity_box", Vec2(1500 + i / 10 * 40, 100 - i % 10 * 40), {
			size = math.random(5, 50) / 10.0
		})
	end
end
