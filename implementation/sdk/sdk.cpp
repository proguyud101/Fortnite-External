#include <implementation/sdk/sdk.hpp>

#include <implementation/utilities/communication/communication.hpp>

#include <implementation/globals.hpp>

#include <implementation/utilities/handler/handler.hpp>

struct rotation_struct
{
	double a;
	char pad_0008[24]; 
	double b; 
	char pad_0028[424];
	double c; 
}; 

bool sdk_::camera_::update()
{
	auto rotation_ptr = communication->read<uint64_t>(cached->gworld + 0x110);
	auto location_ptr = communication->read<uint64_t>(cached->gworld + 0x120);

	auto rotation = communication->read<rotation_struct>(rotation_ptr);
	
	location_v = communication->read<vec_3d>(location_ptr);
	field_of_view_v = communication->read<float>(cached->player_controller + 0x394) * 90.f;
	rotation_v.x = asin(rotation.c) * (180.0 / M_PI);
	rotation_v.y = ((atan2(rotation.a * -1, rotation.b) * (180.0 / M_PI)) * -1) * -1;

	if (location_v.x && field_of_view_v && rotation_v.x != 0) return true;

	else return false;
}



inline float ratio = (float)settings->screenX / settings->screenY;

vec_2d sdk_::camera_::world_to_screen( vec_3d vec3d_location )
{
	quad_matrix matrix_v = matrix(rotation_v, vec_3d(0, 0, 0));

	vec_3d vAxisX = vec_3d(matrix_v.m[0][0], matrix_v.m[0][1], matrix_v.m[0][2]);
	vec_3d vAxisY = vec_3d(matrix_v.m[1][0], matrix_v.m[1][1], matrix_v.m[1][2]);
	vec_3d vAxisZ = vec_3d(matrix_v.m[2][0], matrix_v.m[2][1], matrix_v.m[2][2]);

	vec_3d vDelta = vec3d_location - location_v;

	vec_3d vTransformed = vec_3d(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f) vTransformed.z = 1.f;

	float fov = ratio / (16.0f / 9.0f) * (float)tanf(field_of_view_v * M_PI / 360.0f);

	return vec_2d(settings->centerX + vTransformed.x * ((settings->centerX / fov)) / vTransformed.z, settings->centerY - vTransformed.y * ((settings->centerX / fov)) / vTransformed.z);
}
vec_3d sdk_::camera_::predict(vec_3d target, vec_3d targetVelocity, float projectileSpeed, float projectileGravityScale, float distance)
{
	float horizontalTime = distance / projectileSpeed;
	float verticalTime = distance / projectileSpeed;

	target.x -= verticalTime / 3;
	target.y += targetVelocity.y * horizontalTime;
	target.z -= targetVelocity.z * verticalTime +
		abs(980 * projectileGravityScale) * 0.5f * (verticalTime * verticalTime);

	return target;
}
void sdk_::camera_::aimbot(float x, float y)
{
	float target_x = 0;
	float target_y = 0;

	if (x != 0.f)
	{
		target_x = (x > settings->centerX) ? -(settings->centerX - x) : x - settings->centerX;
		target_x /= settings->aimbot->smoothing;
		target_x = (x > settings->centerX && target_x + settings->centerX > settings->centerX * 2) ? 0 : target_x;
		target_x = (x < settings->centerX && target_x + settings->centerX < 0) ? 0 : target_x;
	}

	if (y != 0.f)
	{
		target_y = (y > settings->centerY) ? -(settings->centerY - y) : y - settings->centerY;
		target_y /= settings->aimbot->smoothing;
		target_y = (y > settings->centerY && target_y + settings->centerY > settings->centerY * 2) ? 0 : target_y;
		target_y = (y < settings->centerY && target_y + settings->centerY < 0) ? 0 : target_y;
	}

	mouse_handler->move(target_x, target_y);
}

vec_3d sdk_::game_::bone_location(actor* pawn, int id)
{
	auto bone_array = communication->read<uintptr_t>(pawn->mesh + 0x10 * communication->read<int>(pawn->mesh + 0x600) + 0x5B8);

	auto component_to_world = communication->read<transform_t>(pawn->mesh + offsets->c2w);

	auto bone = communication->read<transform_t>(bone_array + (id * 0x30));

	quad_matrix matrix = matrix_md(bone.ToMatrixWithScale(), component_to_world.ToMatrixWithScale());

	return vec_3d(matrix._41, matrix._42, matrix._43);
}

std::string sdk_::game_::platform(actor* pawn)
{
	if (!pawn->platform_read)
	{
		wchar_t buffer[64]{ 0 };

		uintptr_t test_platform = communication->read<uintptr_t>(pawn->player_state + 0x438);

		communication->read_memory((PVOID)test_platform, (PVOID)buffer, sizeof(buffer));

		std::wstring platform_wstr(buffer);

		pawn->platform = std::string(platform_wstr.begin(), platform_wstr.end());

		if (pawn->platform.empty()) return "";

		pawn->platform_read = true;

		return std::string(platform_wstr.begin(), platform_wstr.end());
	}
	if (pawn->platform_read) return pawn->platform;
}
void username(actor* pawn)
{
	if (!pawn->name_read)
	{
		pawn->name = "";

		if (!pawn->name.empty()) pawn->name_read = true;
	}
}



