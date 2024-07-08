#include <vector>
#include <string>
#include <stdint.h>
#include <cmath>

#define M_PI 3.14159265358979323846264338327950288419716939937510

class vec_3d {
public:
	float x, y, z;

	// Constructors
	vec_3d() : x(0), y(0), z(0) {}
	vec_3d(float x, float y, float z) : x(x), y(y), z(z) {}

	// Basic vector operations
	vec_3d operator+(const vec_3d& other) const {
		return vec_3d(x + other.x, y + other.y, z + other.z);
	}

	vec_3d operator-(const vec_3d& other) const {
		return vec_3d(x - other.x, y - other.y, z - other.z);
	}

	vec_3d operator*(float scalar) const {
		return vec_3d(x * scalar, y * scalar, z * scalar);
	}

	vec_3d operator/(float scalar) const {
		return vec_3d(x / scalar, y / scalar, z / scalar);
	}

	// Dot product
	float Dot(const vec_3d& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	// Cross product
	vec_3d cross(const vec_3d& other) const {
		return vec_3d(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}

	// Magnitude
	float magnitude() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	// Normalize
	vec_3d normalize() const {
		float mag = magnitude();
		return vec_3d(x / mag, y / mag, z / mag);
	}
	float distance(const vec_3d& other) const {
		return std::sqrt((x - other.x) * (x - other.x) +
			(y - other.y) * (y - other.y) +
			(z - other.z) * (z - other.z));
	}
};

class vec_2d 
{
public:
	float x, y;

	// Constructors
	vec_2d() : x(0), y(0) {}
	vec_2d(float x, float y) : x(x), y(y) {}

	// Basic vector operations
	vec_2d operator+(const vec_2d& other) const {
		return vec_2d(x + other.x, y + other.y);
	}

	vec_2d operator-(const vec_2d& other) const {
		return vec_2d(x - other.x, y - other.y);
	}

	vec_2d operator*(float scalar) const {
		return vec_2d(x * scalar, y * scalar);
	}

	vec_2d operator/(float scalar) const {
		return vec_2d(x / scalar, y / scalar);
	}

	// Dot product
	float dot(const vec_2d& other) const {
		return x * other.x + y * other.y;
	}

	// Magnitude
	float magnitude() const {
		return std::sqrt(x * x + y * y);
	}

	// Normalize
	vec_2d normalize() const {
		float mag = magnitude();
		return vec_2d(x / mag, y / mag);
	}

};

struct Fquat
{
	double x;
	double y;
	double z;
	double w;
};
typedef struct _quad_
{
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
	};
} quad_matrix;
struct transform_t
{
	Fquat rot;
	vec_3d translation;
	vec_3d scale;

	void clear()
	{
		rot = { 0.0f, 0.0f, 0.0f, 1.0f };
		translation = { 0.0f, 0.0f, 0.0f };
		scale = { 1.0f, 1.0f, 1.0f };
	}

	inline quad_matrix ToMatrixWithScale()
	{
		quad_matrix m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};



inline quad_matrix matrix_md(quad_matrix pM1, quad_matrix pM2)
{
	quad_matrix pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

inline quad_matrix matrix(vec_3d rot, vec_3d origin = vec_3d(0, 0, 0))
{
	float radPitch = (rot.x * float(3.14159265358979323846) / 180.f);
	float radYaw = (rot.y * float(3.14159265358979323846) / 180.f);
	float radRoll = (rot.z * float(3.14159265358979323846) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	quad_matrix matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

class actor
{
public:
	uint64_t bone_array;
	transform_t component_to_world;
	bool boneray = false;

	uintptr_t pawn;
	uintptr_t mesh;

	std::string name;
	bool name_read = false;

	std::string platform;
	bool platform_read = false;

	uintptr_t root_component;
	uintptr_t player_state;
	int team_index;
	vec_3d velocity;

	void reset()
	{
		bone_array = 0;
		component_to_world.clear();
		pawn = 0;
		mesh = 0;
		name = "";
		name_read = false;
		root_component = 0;
		player_state = 0;
		team_index = 0;
	}
};

inline std::vector<actor> actor_list;

inline float aimbot_distance;
inline actor aimbot_entity;

class sdk_
{
private:

public:
	class game_ 
	{
	public:
		std::string platform(actor* pawn);
		vec_3d bone_location(actor* pawn, int id);

	}; game_ game_manager;

	class camera_
	{

	public:
		static inline  vec_3d location_v, rotation_v;
		static inline float field_of_view_v;

		static bool update();

		static vec_2d world_to_screen(vec_3d vec3d_location);

		static void aimbot(float x, float y);

		static vec_3d predict(vec_3d target, vec_3d targetVelocity, float projectileSpeed, float projectileGravityScale, float distance);
		
	}; camera_ camera_manager;

};inline sdk_ sdk;
