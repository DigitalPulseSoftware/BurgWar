RegisterClientScript("cl_camera.lua")
RegisterClientScript("cl_init.lua")
RegisterClientScript("cl_scoreboard.lua")
RegisterClientScript("shared.lua")

include("sh_teams.lua")

local gamemode = ScriptedGamemode()
print("Mode de jeu: " .. gamemode.Name)

gamemode.Musics = {
    "Music/mushroomdance_0.ogg"
}

for _, music in pairs(gamemode.Musics) do
    RegisterClientAssets(music)
end
