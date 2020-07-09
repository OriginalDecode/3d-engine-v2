#include "Camera.h"

void Camera::InitOrthographicProjection(float width, float height, float near_plane, float far_plane)
{
	m_ProjectionMatrix = Core::Matrix44f::CreateOrthogonalMatrixLH(width, height, near_plane, far_plane);
}

void Camera::InitOrthogonalProjection(float width, float height, float near_plane, float far_plane)
{
	m_ProjectionMatrix = Core::Matrix44f::CreateOrthogonalMatrixLH(width, height, near_plane, far_plane);
}

void Camera::InitPerspectiveProjection(float width, float height, float near_plane, float far_plane, float fov)
{
	// m_ProjectionMatrix = Core::Matrix44f::CreateProjectionMatrixLH(near_plane, far_plane, width / height, fov);
	m_ProjectionMatrix = Core::VKCreatePerspectiveMatrix(near_plane, far_plane, width / height, fov);
}

void Camera::Update() // called once per frame
{
	m_ViewProjection = m_ProjectionMatrix * Core::FastInverse(m_ViewMatrix);
}

Core::Matrix44f* Camera::GetViewProjectionPointer()
{
	return &m_ViewProjection;
}

void Camera::OrientCamera(const Core::Vector2f& cursor_pos)
{
	m_CenterPoint += cursor_pos * 0.005f;
	// m_CenterPoint.y = fmaxf( fminf( 1.57f, m_CenterPoint.y ), -1.57f ); // limit the view up / down angle to not
	// do a 360 rotation.

	m_Pitch = Core::Quaternion(Core::Vector3f(1.f, 0, 0), m_CenterPoint.y);
	m_Yaw = Core::Quaternion(Core::Vector3f(0, 1.f, 0), m_CenterPoint.x);

	Core::Vector3f axisX(1.f, 0, 0);
	Core::Vector3f axisY(0, 1.f, 0);
	Core::Vector3f axisZ(0, 0, 1.f);

	axisX = m_Yaw * m_Pitch * axisX;
	axisY = m_Yaw * m_Pitch * axisY;
	axisZ = m_Yaw * m_Pitch * axisZ;

	// m_Rotation2 = m_Rotation;

	m_ViewMatrix[0] = axisX.x;
	m_ViewMatrix[1] = axisX.y;
	m_ViewMatrix[2] = axisX.z;

	m_ViewMatrix[4] = axisY.x;
	m_ViewMatrix[5] = axisY.y;
	m_ViewMatrix[6] = axisY.z;

	m_ViewMatrix[8] = axisZ.x;
	m_ViewMatrix[9] = axisZ.y;
	m_ViewMatrix[10] = axisZ.z;
}

void Camera::Forward(float distance)
{
	const Core::Vector4f& t = m_ViewMatrix.GetTranslation();
	const Core::Vector4f& f = m_ViewMatrix.GetForward();

	const Core::Vector4f nt = t + (f * distance);

	m_ViewMatrix.SetTranslation(nt);
}

void Camera::Right(float distance)
{
	m_ViewMatrix.SetTranslation(m_ViewMatrix.GetTranslation() + (m_ViewMatrix.GetRight() * distance));
}

void Camera::Up(float distance)
{
	m_ViewMatrix.SetTranslation(m_ViewMatrix.GetTranslation() + (m_ViewMatrix.GetUp() * distance));
}

void Camera::SetTranslation(const Core::Vector4f& translation)
{
	m_ViewMatrix.SetTranslation(translation);
}
