#pragma once
#include "d3d11.h"
#include <SimpleMath.h>

class Cam
{



	DirectX::SimpleMath::Vector3 m_campos;
	DirectX::SimpleMath::Vector3 m_camor;
	DirectX::SimpleMath::Vector3 m_camlookat;
	DirectX::SimpleMath::Vector3 m_camlookdir;
	DirectX::SimpleMath::Vector3 m_camright;

	float m_camrotrate;

public:

	Cam();
	~Cam();

	void SetPos(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetPos();

	void SetOr(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetOr();

	void SetLookat(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetLookat();

	void SetLookDirection(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetLookDirection();

	void SetCamRight(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetCamRight();

	void SetCamRotationRate(float val);
	float getrotrate();

	void update();
};

