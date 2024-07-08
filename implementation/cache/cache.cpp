#include <thread>
#include <chrono>

#include <implementation/cache/cache.hpp>
#include <implementation/globals.hpp>
#include <implementation/utilities/communication/communication.hpp>

#include <implementation/sdk/sdk.hpp>

void cache_::cache_game()
{
	while (true)
	{
		cached->gworld = communication->read<uint64_t>(communication->GameBase + offsets->gworld);

		cached->game_instance = communication->read<uint64_t>(cached->gworld + offsets->game_instance);

		cached->game_state = communication->read<uint64_t>(cached->gworld + offsets->game_state);

		auto local_players = communication->read<uint64_t>(cached->game_state + offsets->local_players);

		cached->local_player = communication->read<uint64_t>(local_players);

		cached->player_controller = communication->read<uint64_t>(cached->local_player + offsets->player_controller);

		cached->local_pawn = communication->read<uint64_t>(cached->player_controller + offsets->pawn);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void cache_::cache_actors() 
{
	std::vector<actor> temp_stored;
	while (true) 
	{
		auto player_array = communication->read<uint64_t>(cached->game_state + offsets->player_array);

		auto player_array_size = communication->read<int>(cached->game_state + (offsets->player_array + sizeof(uintptr_t)));

		for (int i = 0; i < player_array_size; ++i)
		{
			auto player_state = communication->read<uint64_t>(player_array + (i * sizeof(uintptr_t)));
			if (!player_state) continue;

			auto pawn = communication->read<uint64_t>(player_state + offsets->pawn);
			if (!pawn) continue;

			auto mesh = communication->read<uint64_t>(pawn + offsets->mesh);
			if (!mesh) continue;

			actor stored;

			stored.pawn = pawn;
			stored.mesh = mesh;
			stored.player_state = player_state;

			temp_stored.push_back(stored);

		}
		std::swap(actor_list, temp_stored);
		temp_stored.clear();

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}