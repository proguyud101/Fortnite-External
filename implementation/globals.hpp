#include <stdint.h>

class settings_
{
public:
	int screenX = 1920, screenY = 1080, centerX = 960, centerY = 540;

	class aimbot_ 
	{
	public:
		bool enabled = true;

		bool prediction = true;
		bool team_check = true;


		int hitbox = 0;

		float smoothing = 5;
		float fov = 300;

	}; aimbot_* aimbot = new aimbot_;

	class visuals_
	{
	public:
		bool enabled = true;

		bool box = true;
		bool distance = true;
		bool skeleton = true;
		bool team_check = true;

		int maximum_distance = 300;
		int minimum_distnace = 0;

		bool platform = false;

	}; visuals_* visuals = new visuals_;

}; inline settings_* settings = new settings_;

class offsets_
{
public:
	int
		gworld = 0x12BA7050,
		game_state = 0x160,
		game_instance = 0x1D8,
		root_component = 0x198,
		local_players = 0x38,
		player_controller = 0x30,
		team_index = 0x1211,
		pawn = 0x338,
		mesh = 0x318,
		c2w = 0x1c0,
		last_submit = 0x000,
		last_render = 0x000,

		player_state = 0x2b0,
		player_array = 0x2a8;

}; inline offsets_* offsets = new offsets_;

class cached_
{
public:
	uint64_t
		gworld,
		game_state,
		game_instance,
		root_component,
		local_player,
		player_controller,
		location_ptr,
		rotation_ptr,
		local_pawn;

	int team_index;

}; inline cached_* cached = new cached_;