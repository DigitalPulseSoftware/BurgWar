function GM:OnPlayerDeath(player, attacker)
	print(player:GetName() .. " died")
	timer.Sleep(2000)
	player:Spawn()
end

function GM:OnPlayerJoin(player)
	print(player:GetName() .. " joined")
	player:Spawn()
end

function GM:OnTick()
end

function GM:OnInit()
	print(self, "Le match a été créé")

	for i = 0, 20 do
		self:CreateEntity("entity_box", Vec2(math.random(0, 10000), -200), {
			size = math.random(10, 20) / 10.0
		})
	end
end
