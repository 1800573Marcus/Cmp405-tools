#include "Cam.h"



Cam::Cam()
{
}


Cam::~Cam()
{
}

float Cam::getrotrate() {
	return m_camrotrate;
}

void Cam::SetCamRotationRate(float val) {
	m_camrotrate = val;
}
void Cam::SetCamRight(DirectX::SimpleMath::Vector3 val) {
	m_camright = val;
}
DirectX::SimpleMath::Vector3 Cam::GetCamRight() {
	return m_camright;
}

void Cam::SetLookDirection(DirectX::SimpleMath::Vector3 val) {
	m_camlookdir = val;
}
DirectX::SimpleMath::Vector3 Cam::GetLookDirection() {
	return m_camlookdir;
}

void Cam::SetLookat(DirectX::SimpleMath::Vector3 val) {
	m_camlookat = val;
}
DirectX::SimpleMath::Vector3 Cam::GetLookat() {
	return m_camlookat;
}
void Cam::SetPos(DirectX::SimpleMath::Vector3 val) {
	m_campos = val;
}
DirectX::SimpleMath::Vector3 Cam::GetPos() {
	return m_campos;
}
void Cam::SetOr(DirectX::SimpleMath::Vector3 val) {
	m_camor = val;
}
DirectX::SimpleMath::Vector3 Cam::GetOr() {
	return m_camor;
}

void Cam::update() {
	//improved camera controls as described in lab work
	m_camlookdir.x = cos((m_camor.y)*3.1415 / 180) * cos((m_camor.x)*3.1415 / 180);
	m_camlookdir.y = sin((m_camor.x)*3.1415 / 180);
	m_camlookdir.z = sin((m_camor.y)*3.1415 / 180) * cos((m_camor.x)*3.1415 / 180);
	m_camlookdir.Normalize();
}