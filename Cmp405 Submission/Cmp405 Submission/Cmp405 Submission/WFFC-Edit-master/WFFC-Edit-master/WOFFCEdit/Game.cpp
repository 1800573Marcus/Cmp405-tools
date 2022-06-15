//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"
#include <string>


using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game()

{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
	m_displayList.clear();
	
	//initial Settings
	//modes
	m_grid = false;

	wireframe = false;

	//functional
	m_movespeed = 0.30;
	cam.SetCamRotationRate(3.0);

	//camera
	DirectX::SimpleMath::Vector3 start(0, 0, 0);

	DirectX::SimpleMath::Vector3 start2(0, 400, 1);

	DirectX::SimpleMath::Vector3 start3(0, 1, 400);

	//tart points for secondary cameras

	cam2.SetPos(start2);
	cam3.SetPos(start3);


	cam.SetOr(start);
	cam.SetLookDirection(start);

	cam.SetOr(start);
	
	


	
	cam.SetLookat(start);
	


	cam.SetLookDirection(start);




	cam.SetCamRight(start);


	
	start.x = 0.0;
	start.y = 3.7;
	start.z = -3.5;

	cam.SetPos(start);

	//camera control
		clicked = false;
}

Game::~Game()
{

#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	camselection = 1;
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	GetClientRect(window, &m_ScreenDimensions);

#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

void Game::SetGridState(bool state)
{
	m_grid = state;
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick(InputCommands *Input)
{
	//copy over the input commands so we have a local version to use elsewhere.
	m_InputCommands = *Input;
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	//TODO  any more complex than this, and the camera should be abstracted out to somewhere else
	//camera motion is on a plane, so kill the 7 component of the look direction
	Vector3 planarMotionVector = cam.GetLookDirection();
	planarMotionVector.y = 0.0;

	Vector3 up(0, 5, 0);//up vector for camera movement


	Vector3 scaler  (0.1, 0.1, 0.1);// controls scaling of selected objects
	Vector3 minscaler(0, 0, 0);// contols minimum possible scaling value

	Vector3 orientate = cam.GetOr();
	if (m_InputCommands.rotRight)
	{
		orientate.y = orientate.y + cam.getrotrate();
		cam.SetOr(orientate);// turn camera right
	}
	if (m_InputCommands.rotLeft)
	{
		orientate.y = orientate.y - cam.getrotrate();

		cam.SetOr(orientate);//turn camera left

	}if (m_InputCommands.rotUp) {//rotate up
		orientate.x = orientate.x + cam.getrotrate();
		cam.SetOr(orientate);
	}if (m_InputCommands.rotDown) {//rotate down
		orientate.x = orientate.x - cam.getrotrate();
		cam.SetOr(orientate);
	}if (m_InputCommands.cam1) {//set camera based on input
		camselection = 1;
	}if (m_InputCommands.cam2) {
		camselection = 2;
	}if (m_InputCommands.cam3) {
		camselection = 3;

	}if (m_InputCommands.wireframe) { //turn wireframe on and off
		if (wireframe == false) {
			wireframe = true;
		}else if (wireframe == true) {
			wireframe = false;
		}
	}

	if (clicked == false) {
		if (m_InputCommands.clicked)// get mouse coordinates when initially clicked
		{
			GetCursorPos(&CursorPoint);
			mousestartx = CursorPoint.x;
			mousestarty = CursorPoint.y;
		}
		clicked = true;
	}
	if (m_InputCommands.mouseup) {
		clicked = false; //set mouse button up
		
	}
	if (m_InputCommands.clicked)
	{
		GetCursorPos(&CursorPoint); // gets mouse coords constantly while down

		if (CursorPoint.x > mousestartx)// if the mouse has moved while the button is down the camera will rotate in that direction
		{
			orientate.y = orientate.y + cam.getrotrate();//if mouse moved right rotate rigt
			cam.SetOr(orientate);
		}
		 if (CursorPoint.x < mousestartx)//if mouse moved left rotate that way
		{
			orientate.y = orientate.y - cam.getrotrate();

			cam.SetOr(orientate);
		}
		 if (CursorPoint.y > (mousestarty - 10))// if mouse moved down rotate down
		{
			orientate.x = orientate.x - (cam.getrotrate()/5);//up and down rotate slower  it is less disorientating
			cam.SetOr(orientate);
		}
		 if (CursorPoint.y < (mousestarty + 10))// if mouse moved up, rotated up
		{
			orientate.x = orientate.x + (cam.getrotrate()/5);

			cam.SetOr(orientate);
		}
	}
	

	if (selectedID != -1) {// while an object is selectefd

		if (m_InputCommands.I) {//moves selected object along X+
			m_displayList[selectedID].m_position.x = m_displayList[selectedID].m_position.x + 0.5;

		}if (m_InputCommands.K) {//moves selected object along X-
			m_displayList[selectedID].m_position.x = m_displayList[selectedID].m_position.x - 0.5;



		}if (m_InputCommands.U) {//moves selected object along Y+
			m_displayList[selectedID].m_position.y = m_displayList[selectedID].m_position.y + 0.5;



		}if (m_InputCommands.O) {//moves selected object along Y-
			m_displayList[selectedID].m_position.y = m_displayList[selectedID].m_position.y - 0.5;


		}if (m_InputCommands.J) {//moves selected object along Z+
			m_displayList[selectedID].m_position.z = m_displayList[selectedID].m_position.z - 0.5;



		}if (m_InputCommands.L) {//moves selected object along Z-
			m_displayList[selectedID].m_position.z = m_displayList[selectedID].m_position.z + 0.5;


		}if (m_InputCommands.scaleup) {
			m_displayList[selectedID].m_scale = m_displayList[selectedID].m_scale + scaler;// scales object up

		}if (m_InputCommands.scaledown) {
			if (m_displayList[selectedID].m_scale.x > minscaler.x) {// scales object down to a minimum of 0
				m_displayList[selectedID].m_scale = m_displayList[selectedID].m_scale - scaler;
			}

		}

		// update scene visuals
			UpdateGraph->at(selectedID).posX = m_displayList[selectedID].m_position.x;
			UpdateGraph->at(selectedID).posY = m_displayList[selectedID].m_position.y;
			UpdateGraph->at(selectedID).posZ = m_displayList[selectedID].m_position.z;


			UpdateGraph->at(selectedID).rotX = m_displayList[selectedID].m_orientation.x;
			UpdateGraph->at(selectedID).rotY = m_displayList[selectedID].m_orientation.y;
			UpdateGraph->at(selectedID).rotZ = m_displayList[selectedID].m_orientation.z;

			UpdateGraph->at(selectedID).scaX = m_displayList[selectedID].m_scale.x;
			UpdateGraph->at(selectedID).scaY = m_displayList[selectedID].m_scale.y;
			UpdateGraph->at(selectedID).scaZ = m_displayList[selectedID].m_scale.z;

			UpdateGraph->at(selectedID).editor_render = m_displayList[selectedID].m_render;
			UpdateGraph->at(selectedID).editor_wireframe = m_displayList[selectedID].m_wireframe;


			UpdateGraph->at(selectedID).light_type = m_displayList[selectedID].m_light_type;
			UpdateGraph->at(selectedID).light_diffuse_r = m_displayList[selectedID].m_light_diffuse_r;
			UpdateGraph->at(selectedID).light_diffuse_g = m_displayList[selectedID].m_light_diffuse_g;
			UpdateGraph->at(selectedID).light_diffuse_b = m_displayList[selectedID].m_light_diffuse_b;
			UpdateGraph->at(selectedID).light_specular_r = m_displayList[selectedID].m_light_specular_r;
			UpdateGraph->at(selectedID).light_specular_g = m_displayList[selectedID].m_light_specular_g;
			UpdateGraph->at(selectedID).light_specular_b = m_displayList[selectedID].m_light_specular_b;
			UpdateGraph->at(selectedID).light_spot_cutoff = m_displayList[selectedID].m_light_spot_cutoff;
			UpdateGraph->at(selectedID).light_constant = m_displayList[selectedID].m_light_constant;
			UpdateGraph->at(selectedID).light_linear = m_displayList[selectedID].m_light_linear;
			UpdateGraph->at(selectedID).light_quadratic = m_displayList[selectedID].m_light_quadratic;

			lookselect.x = m_displayList[selectedID].m_position.x - cam.GetPos().x ;
			lookselect.y = m_displayList[selectedID].m_position.y -  cam.GetPos().y;
			lookselect.z = m_displayList[selectedID].m_position.z - cam.GetPos().z ;
			

			//arc ball, works similar to gravity
			// camera cosntantly moves towards the selected object while also being pushed away. balanced forces make the camera stay a constant distance
				lookselect /= 4;
				cam.SetPos(cam.GetPos() + lookselect);
				cam.SetPos(cam.GetPos() - cam.GetLookDirection()*m_movespeed*2);
			
				//point camera at selected object
			cam.SetLookDirection(lookselect);
			cam.SetLookat(m_displayList[selectedID].m_position);
	}
	

	
	if (camselection == 1) {//update camera based on selectd cam

		cam.update();

		//create right vector from look Direction

		Vector3 crossdir = cam.GetLookDirection();
		crossdir.Cross(up, cam.GetLookDirection());

		cam.SetCamRight(crossdir);




		//process input and update stuff
		if (m_InputCommands.forward)
		{
			cam.SetPos(cam.GetPos() + cam.GetLookDirection()*m_movespeed);
		}
		if (m_InputCommands.back)
		{
			cam.SetPos(cam.GetPos() - cam.GetLookDirection()*m_movespeed);
			//m_camPosition -= m_camLookDirection*m_movespeed;
		}
		if (m_InputCommands.up)
		{
			cam.SetPos(cam.GetPos() + up * m_movespeed);
			//m_camPosition += m_camRight*m_movespeed;
		}
		if (m_InputCommands.down)
		{
			cam.SetPos(cam.GetPos() - up * m_movespeed);
			//m_camPosition -= m_camRight*m_movespeed;
		}

		//update lookat point
		cam.SetLookat(cam.GetPos() + cam.GetLookDirection());
		//m_camLookAt = m_camPosition + m_camLookDirection;

		//apply camera vectors


		m_view = Matrix::CreateLookAt(cam.GetPos(), cam.GetLookat(), Vector3::UnitY);
	}if (camselection == 2) {
		m_view = Matrix::CreateLookAt(cam2.GetPos(), cam2.GetLookat(), Vector3::UnitY);

	}if (camselection == 3) {
		m_view = Matrix::CreateLookAt(cam3.GetPos(), cam3.GetLookat(), Vector3::UnitY);

	}

    m_batchEffect->SetView(m_view);
    m_batchEffect->SetWorld(Matrix::Identity);
	m_displayChunk.m_terrainEffect->SetView(m_view);
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

   
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}
	//CAMERA POSITION ON HUD
	m_sprites->Begin();
	WCHAR   Buffer[256];
	std::wstring var = L"Cam X: " + std::to_wstring(m_camPosition.x) + L"Cam Z: " + std::to_wstring(m_camPosition.z);
	m_font->DrawString(m_sprites.get(), var.c_str() , XMFLOAT2(100, 10), Colors::Yellow);
	m_sprites->End();

	//RENDER OBJECTS FROM SCENEGRAPH
	int numRenderObjects = m_displayList.size();
	for (int i = 0; i < numRenderObjects; i++)
	{
		m_deviceResources->PIXBeginEvent(L"Draw model");
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x, m_displayList[i].m_scale.y, m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x, m_displayList[i].m_position.y, m_displayList[i].m_position.z };

		//convert degrees into radians for rotation matrix
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y *3.1415 / 180,
															m_displayList[i].m_orientation.x *3.1415 / 180,
															m_displayList[i].m_orientation.z *3.1415 / 180);

		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		m_displayList[i].m_model->Draw(context, *m_states, local, m_view, m_projection, wireframe);	//last variable in draw,  make TRUE for wireframe

		if (i == selectedID)
		{
			m_displayList[i].m_model->UpdateEffects([&](IEffect* effect)
			{
				auto fog = dynamic_cast<IEffectFog*>(effect);
				if (fog)
				{
					fog->SetFogEnabled(true);
					fog->SetFogStart(0); // assuming RH coordiantes
					fog->SetFogEnd(8);
					fog->SetFogColor(Colors::Gold);
				}
			});
		}
		else
		{
			m_displayList[i].m_model->UpdateEffects([&](IEffect* effect)
			{
				auto fog = dynamic_cast<IEffectFog*>(effect);
				if (fog)
				{
					fog->SetFogEnabled(false);
				}
			});
		}

		m_deviceResources->PIXEndEvent();
	}
    m_deviceResources->PIXEndEvent();

	//RENDER TERRAIN
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(),0);
	context->RSSetState(m_states->CullNone());

	if (wireframe == true) {
		context->RSSetState(m_states->Wireframe());		// wireframe
	}
	//Render the batch,  This is handled in the Display chunk becuase it has the potential to get complex
	m_displayChunk.RenderBatch(m_deviceResources);

    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
    m_deviceResources->PIXBeginEvent(L"Draw grid");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullCounterClockwise());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::BuildDisplayList(std::vector<SceneObject> * SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

	if (!m_displayList.empty())		//is the vector empty
	{
		m_displayList.clear();		//if not, empty it
	}

	//for every item in the scenegraph
	int numObjects = SceneGraph->size();
	for (int i = 0; i < numObjects; i++)
	{
		
		//create a temp display object that we will populate then append to the display list.
		DisplayObject newDisplayObject;
		
		//load model
		std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);							//convect string to Wchar
		newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)

		//Load Texture
		std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);								//convect string to Wchar
		HRESULT rs;
		rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

		//if texture fails.  load error default
		if (rs)
		{
			CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
		}

		//apply new texture to models effect
		newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) //This uses a Lambda function,  if you dont understand it: Look it up.
		{	
			auto lights = dynamic_cast<BasicEffect*>(effect);
			if (lights)
			{
				lights->SetTexture(newDisplayObject.m_texture_diffuse);			
			}
		});

		//set position
		newDisplayObject.m_position.x = SceneGraph->at(i).posX;
		newDisplayObject.m_position.y = SceneGraph->at(i).posY;
		newDisplayObject.m_position.z = SceneGraph->at(i).posZ;
		
		//setorientation
		newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
		newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
		newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

		//set scale
		newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
		newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
		newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

		//set wireframe / render flags
		newDisplayObject.m_render		= SceneGraph->at(i).editor_render;
		newDisplayObject.m_wireframe	= SceneGraph->at(i).editor_wireframe;

		newDisplayObject.m_light_type		= SceneGraph->at(i).light_type;
		newDisplayObject.m_light_diffuse_r	= SceneGraph->at(i).light_diffuse_r;
		newDisplayObject.m_light_diffuse_g	= SceneGraph->at(i).light_diffuse_g;
		newDisplayObject.m_light_diffuse_b	= SceneGraph->at(i).light_diffuse_b;
		newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
		newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
		newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
		newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
		newDisplayObject.m_light_constant	= SceneGraph->at(i).light_constant;
		newDisplayObject.m_light_linear		= SceneGraph->at(i).light_linear;
		newDisplayObject.m_light_quadratic	= SceneGraph->at(i).light_quadratic;
		
		m_displayList.push_back(newDisplayObject);
		
	}
		
	UpdateGraph = SceneGraph;
	
}

void Game::BuildDisplayChunk(ChunkObject * SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
}

void Game::SaveDisplayChunk(ChunkObject * SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif


#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_fxFactory = std::make_unique<EffectFactory>(device);
	m_fxFactory->SetDirectory(L"database/data/"); //fx Factory will look in the database directory
	m_fxFactory->SetSharing(false);	//we must set this to false otherwise it will share effects based on the initial tex loaded (When the model loads) rather than what we will change them to.

    m_sprites = std::make_unique<SpriteBatch>(context);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

//    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(device, L"tiny.sdkmesh", *m_fxFactory);
	

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
    );

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_batchEffect->SetProjection(m_projection);
	
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();
    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

std::wstring StringToWCHART(std::string s)
{

	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

int Game::MousePicking()
{
	 selectedID = -1;

	float pickedDistance = 0;

	//setup near and far planes of frustum with mouse X and mouse y passed down from Toolmain. 
	//they may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 1.0f, 1.0f);

	//Loop through entire display list of objects and pick with each in turn. 
	for (int i = 0; i < m_displayList.size(); i++)
	{
		//Get the scale factor and translation of the object
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x,		m_displayList[i].m_scale.y,		m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x,		m_displayList[i].m_position.y,	m_displayList[i].m_position.z };

		//convert euler angles into a quaternion for the rotation of the object
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y *3.1415 / 180, m_displayList[i].m_orientation.x *3.1415 / 180,
			m_displayList[i].m_orientation.z *3.1415 / 180);

		//create set the matrix of the selected object in the world based on the translation, scale and rotation.
		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		//Unproject the points on the near and far plane, with respect to the matrix we just created.
		XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);

		XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);

		//turn the transformed points into our picking vector. 
		XMVECTOR pickingVector = farPoint - nearPoint;
		pickingVector = XMVector3Normalize(pickingVector);

		//loop through mesh list for object
		for (int y = 0; y < m_displayList[i].m_model.get()->meshes.size(); y++)
		{
			//checking for ray intersection
			if (m_displayList[i].m_model.get()->meshes[y]->boundingBox.Intersects(nearPoint, pickingVector, pickedDistance))
			{
				selectedID = i;
			}
		}
	}
	//if we got a hit.  return it.  
	return selectedID;

}
