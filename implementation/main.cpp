#include <windows.h>
#include <iostream>
#include <stdint.h>
#include <string>
#include <thread>
#include <Urlmon.h>
#pragma comment(lib, "Urlmon.lib")

#include <implementation/utilities/communication/communication.hpp>

#include <implementation/utilities/authentication/authentication.hpp>

#include <implementation/render/render.hpp>
#include <implementation/sdk/sdk.hpp>
#include <implementation/globals.hpp>
#include <implementation/cache/cache.hpp>

#include <implementation/utilities/handler/handler.hpp>

#include <implementation/utilities/mapper/mapper.hpp>

std::string name = ("name");
std::string ownerid = ("ownerid");
std::string secret = ("secret");
std::string version = ("1.0");
std::string url = ("https://keyauth.win/api/1.2/"); // change if you're self-hosting
std::string path = (""); //optional, set a path if you're using the token validation setting

KeyAuth::api CustomApi(name, ownerid, secret, version, url, path);
std::string key;

int main()
{
	CustomApi.init();

	std::cout << " [+]  enter key -> ";
	std::cin >> key;
	CustomApi.license(key);

	if (!CustomApi.response.success) exit(0);

	mouse_handler->load_win32u();
	window_handler->load_user32();
	window_handler->load_dwmapi();

	std::thread(cache->cache_actors).detach();
	std::thread(cache->cache_game).detach();

	render.setup();

	render.init();

	render.render();
}