#include <implementation/render/render.hpp>

#include <implementation/sdk/sdk.hpp>
#include <windows.h>
#include <dwmapi.h>

#include <implementation/globals.hpp>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx11tex.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")

#include <dependencies/imgui/imgui.h>
#include <dependencies/imgui/imgui_impl_win32.h>
#include <dependencies/imgui/imgui_impl_dx11.h>

HWND handle;
ID3D11Device* d3d_device;
ID3D11DeviceContext* d3d_device_ctx;
IDXGISwapChain* d3d_swap_chain;
ID3D11RenderTargetView* d3d_render_target;

//hooks our overlay
bool render_::setup()
{
	handle = FindWindowA(("CEF-OSC-WIDGET"), ("NVIDIA GeForce Overlay"));

	if (!handle) return false;
	MARGINS Margin = { -1 };

	(ShowWindow)(handle, SW_SHOW);

	(SetWindowLongA)(handle, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	(SetWindowLongA)(
		handle,
		-20,
		static_cast<LONG_PTR>(
			static_cast<int>((GetWindowLongA)(handle, -20)) | 0x20
			)
		);

	//transparency
	MARGINS margin = { -1, -1, -1, -1 };
	(DwmExtendFrameIntoClientArea)(
		handle,
		&margin
		);

	(SetLayeredWindowAttributes)(
		handle,
		NULL,
		0xFF,
		0x02
		);

	// top most
	(SetWindowPos)(
		handle,
		HWND_TOPMOST,
		0, 0, 0, 0,
		0x0002 | 0x0001
		);


	(UpdateWindow)(handle);

	return true;
}

//init all imgui and d3d11
bool render_::init()
{
	DXGI_SWAP_CHAIN_DESC swap_chain_description = {};
	swap_chain_description.BufferCount = 2;
	swap_chain_description.BufferDesc.Width = 0; // Set the appropriate width based on your requirements
	swap_chain_description.BufferDesc.Height = 0; // Set the appropriate height based on your requirements
	swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_description.OutputWindow = handle;
	swap_chain_description.SampleDesc.Count = 1;
	swap_chain_description.SampleDesc.Quality = 0;
	swap_chain_description.Windowed = TRUE;
	swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL d3d_feature_lvl;

	const D3D_FEATURE_LEVEL d3d_feature_array[1] = { D3D_FEATURE_LEVEL_11_0 };

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		d3d_feature_array,
		1,
		D3D11_SDK_VERSION,
		&swap_chain_description,
		&d3d_swap_chain,
		&d3d_device,
		&d3d_feature_lvl,
		&d3d_device_ctx);

	ID3D11Texture2D* pBackBuffer;

	d3d_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

	d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &d3d_render_target);

	pBackBuffer->Release();

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui_ImplWin32_Init(&handle);

	ImGui_ImplDX11_Init(d3d_device, d3d_device_ctx);

	d3d_device->Release();

	return true;
}


void render_players()
{
	//reset after aimboting so that we dont aimbot someone that isnt in our fov anymore
	aimbot_entity.reset();

	//reset aimbot distance to high asf number
	aimbot_distance = FLT_MAX;

	for (auto& entity : actor_list)
	{
		//getting head 3d then converting to 2d for drawing on screen
		auto head_3d = sdk.game_manager.bone_location(&entity, 110);
		auto head_2d = sdk.camera_manager.world_to_screen(head_3d);

		//A little bit higher then head so box looks better
		auto head_box = sdk.camera_manager.world_to_screen(vec_3d(head_3d.x, head_3d.y, head_3d.z + 15));

		//getting root / feet 3d then converting to 2d for drawing on screen
		auto root_3d = sdk.game_manager.bone_location(&entity, 0);
		auto root_2d = sdk.camera_manager.world_to_screen(root_3d);

		int distance = head_3d.distance(sdk.camera_manager.location_v) / 100;

		if (settings->visuals->enabled)
		{
			if (settings->visuals->box)
			{
				float box_height = abs(head_box.y - root_2d.y);
				float box_width = abs(head_2d.y - root_2d.y) * 0.50f;

				ImGui::GetBackgroundDrawList()->AddRect(ImVec2(head_box.x - (box_width / 2), head_box.y), ImVec2(head_box.x - (box_width / 2) + box_width, head_box.y + box_height), ImColor(255, 255, 255), 3.0f, 0, 2.0f);
			}
			if (settings->visuals->platform)
			{
				auto size = ImGui::CalcTextSize(entity.platform.c_str());

				ImGui::GetBackgroundDrawList()->AddText(ImVec2((root_2d.x - size.x / 2), root_2d.y + 15), ImColor(255, 255, 255), entity.platform.c_str());
			}
			if (settings->visuals->distance)
			{
				//combine distance with M
				std::string distance_str = "{ " + std::to_string(distance) + " } M";

				//calculate the size of the string so that we can take away half of the size.x so that the distance will be perfectly centered
				auto size = ImGui::CalcTextSize(distance_str.c_str());

				//add the text to draw list whith out color and location which is white and the feet
				ImGui::GetBackgroundDrawList()->AddText(ImVec2((root_2d.x - size.x / 2), root_2d.y), ImColor(255, 255, 255), distance_str.c_str());
			}
		}
		//calculates the distance in pixels to the player from crosshair and if the distance is smaller then the distance to edge of fov circle then passthrough the current pawn to the aimbot
		auto dx = head_2d.x - (settings->screenX / 2);
		auto dy = head_2d.y - (settings->screenY / 2);
		auto dist = sqrtf(dx * dx + dy * dy);

		if (dist < settings->aimbot->fov && dist < aimbot_distance)
		{
			//passes through our current pawn to the aimbot by setting aimbot entity
			aimbot_entity = entity;
			//adds the distance to the entity from crosshair so if the next pawn is closer then we will aimbot them instead
			aimbot_distance = dist;
		}
	}
	//checking if there is any entities to aimboy
	if (!aimbot_entity.mesh) return;

	auto head_3d = sdk.game_manager.bone_location(&aimbot_entity, 110);

	//if you want you can do prediction here using head_3d and then we world to screen and aimbot to head_2d

	head_3d = sdk.camera_manager.predict(head_3d, velocity, etc);

	auto head_2d = sdk.camera_manager.world_to_screen(head_3d);

	sdk.camera_manager.aimbot(head_2d.x, head_2d.y);
}

void render_loop()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.f);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	render_players();

	//get current fps of our render
	int fps = ImGui::GetIO().Framerate;

	//combine are trademark and fps
	std::string water_mark = "Sytex External FPS -> " + std::to_string(fps);

	//draw combined so we see fps and Syntex External
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(15, 15), ImColor(255, 255, 255), water_mark.c_str());


	//render everything we have added to render list
	ImGui::Render();

	//clear everything to black so we dont see boxes of people who arent there
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	d3d_device_ctx->OMSetRenderTargets(1, &d3d_render_target, nullptr);
	d3d_device_ctx->ClearRenderTargetView(d3d_render_target, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//present our draw data from imgui to our overlay
	d3d_swap_chain->Present(1, 0);
}
bool render_::render()
{
	static RECT rect_og;
	MSG msg = { NULL };
	ZeroMemory(&msg, sizeof(MSG));

	while (msg.message != WM_QUIT)
	{
		UpdateWindow(handle);
		ShowWindow(handle, SW_SHOW);

		if (PeekMessageA(&msg, handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = 1.0f / 60.0f;

		POINT p_cursor;
		GetCursorPos(&p_cursor);
		io.MousePos.x = p_cursor.x;
		io.MousePos.y = p_cursor.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		render_loop();

	}
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(handle);

	return true;
}
