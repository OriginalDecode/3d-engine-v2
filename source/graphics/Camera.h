#pragma once
#include "Core/math/Matrix44.h"
#include "Core/math/Quaternion.h"
#include "Core/math/Vector2.h"
class Camera
{
public:
	Camera() = default;
	~Camera() = default;

	void InitOrthographicProjection(float width, float height, float near_plane, float far_plane);
	void InitOrthogonalProjection(float width, float height, float near_plane, float far_plane);
	void InitPerspectiveProjection(float width, float height, float near_plane, float far_plane, float fov);

	void Update();

	Core::Matrix44f* GetViewProjectionPointer();

	Core::Matrix44f* GetView() { return &m_ViewMatrixInverse; }
	Core::Matrix44f* GetProjection() { return &m_ProjectionMatrix; }
	Core::Matrix44f* GetViewProjection() { return &m_ViewProjection; }

	void OrientCamera(const Core::Vector2f& cursor_pos);

	void Forward(float distance);
	void Right(float distance);
	void Up(float distance);

	void SetTranslation(const Core::Vector4f& translation);

	const Core::Vector4f& GetPosition() { return m_ViewMatrix.GetTranslation(); }

private:
	Core::Matrix44f m_ProjectionMatrix = Core::Matrix44f::Identity();
	Core::Matrix44f m_ViewMatrix = Core::Matrix44f::Identity();
	Core::Matrix44f m_ViewMatrixInverse = Core::Matrix44f::Identity();
	Core::Matrix44f m_ViewProjection = Core::Matrix44f::Identity();

	Core::Vector2f m_CenterPoint;
	Core::Quaternion m_Pitch;
	Core::Quaternion m_Yaw;
	Core::Quaternion m_Roll;
};
