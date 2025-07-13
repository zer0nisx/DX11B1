#include <Windows.h>
#include <memory>
#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "Renderer/D3D11Renderer.h"
#include "Renderer/Texture.h"
#include "Renderer/Light.h"
#include "Renderer/ShadowMap.h"

using namespace GameEngine;

class TestGame : public Core::Engine {
public:
	TestGame() : m_rotationY(0.0f), m_currentTexture(0), m_lightingEnabled(true), m_shadowsEnabled(true) {}

protected:
	bool OnInitialize() override {
		LOG_INFO("TestGame initializing...");

		// Create test scene geometry
		if (!CreateSceneGeometry()) {
			LOG_ERROR("Failed to create scene geometry");
			return false;
		}

		// Load shaders
		if (!LoadShaders()) {
			LOG_ERROR("Failed to load shaders");
			return false;
		}

		// Create textures
		if (!CreateTextures()) {
			LOG_ERROR("Failed to create textures");
			return false;
		}

		// Initialize lighting system
		if (!InitializeLights()) {
			LOG_ERROR("Failed to initialize lighting system");
			return false;
		}

		// Set up camera
		SetCameraPosition(Math::Vector3(0.0f, 3.0f, -8.0f));
		SetCameraTarget(Math::Vector3(0.0f, 0.0f, 0.0f));

		LOG_INFO("TestGame initialized successfully");
		LOG_INFO("Controls:");
		LOG_INFO("  WASD - Camera movement");
		LOG_INFO("  R - Reset rotation");
		LOG_INFO("  C - Create new cube");
		LOG_INFO("  T - Toggle texture");
		LOG_INFO("  L - Toggle lighting");
		LOG_INFO("  S - Toggle shadows");
		LOG_INFO("  1,2,3 - Switch light types");
		LOG_INFO("  Space - Debug info");
		return true;
	}

	void OnUpdate(float deltaTime) override {
		// Rotate the cube
		m_rotationY += deltaTime * 45.0f; // 45 degrees per second
		if (m_rotationY > 360.0f) {
			m_rotationY -= 360.0f;
		}

		// Update lighting system
		UpdateLights(deltaTime);

		// Handle additional input
		static bool spacePressed = false;
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			if (!spacePressed) {
				LogDebugInfo();
				spacePressed = true;
			}
		}
		else {
			spacePressed = false;
		}
	}

	void OnRender() override {
		auto& renderer = GetRenderer();

		// Set shaders
		renderer.SetVertexShader(m_vertexShader.Get(), m_inputLayout.Get());
		renderer.SetPixelShader(m_pixelShader.Get());

		// Set constant buffer
		renderer.SetConstantBuffer(renderer.GetDevice() ? m_constantBuffer.Get() : nullptr, 0);

		// Set texture and sampler
		if (m_currentTexture == 0 && m_checkerTexture) {
			renderer.SetTexture(m_checkerTexture->GetSRV(), 0);
		}
		else if (m_currentTexture == 1 && m_colorTexture) {
			renderer.SetTexture(m_colorTexture->GetSRV(), 0);
		}

		if (m_samplerState) {
			renderer.SetSampler(m_samplerState.Get(), 0);
		}

		// Update lighting if enabled
		if (m_lightingEnabled) {
			renderer.UpdateLightBuffer(renderer.GetLightManager(), GetCameraPosition());
		}

		// Render scene
		RenderScene();
	}

	void OnShutdown() override {
		LOG_INFO("TestGame shutting down...");

		// Clean up geometry
		m_testCube.reset();
		m_groundPlane.reset();

		// Clean up shaders
		m_vertexShader.Reset();
		m_pixelShader.Reset();
		m_inputLayout.Reset();
		m_constantBuffer.Reset();

		// Clean up textures
		m_checkerTexture.reset();
		m_colorTexture.reset();
		m_samplerState.Reset();

		LOG_INFO("TestGame shutdown complete");
	}

	void OnKeyboard(int key, bool isDown) override {
		// Call base class implementation first
		Core::Engine::OnKeyboard(key, isDown);

		if (!isDown) return;

		switch (key) {
		case 'R':
			m_rotationY = 0.0f;
			LOG_INFO("Rotation reset");
			break;

		case 'C':
		{
			// Create a new cube
			m_testCube = Mesh::Mesh::CreateCube(&GetRenderer(),
				1.0f + (rand() % 100) / 100.0f);
			LOG_INFO("Created new cube");
		}
		break;

		case 'T':
			// Toggle texture
			m_currentTexture = (m_currentTexture + 1) % 2;
			LOG_INFO("Switched to texture " << m_currentTexture);
			break;

		case 'L':
			// Toggle lighting
			m_lightingEnabled = !m_lightingEnabled;
			LOG_INFO("Lighting " << (m_lightingEnabled ? "enabled" : "disabled"));
			break;

		case 'S':
			// Toggle shadows
			m_shadowsEnabled = !m_shadowsEnabled;
			ToggleShadows();
			LOG_INFO("Shadows " << (m_shadowsEnabled ? "enabled" : "disabled"));
			break;

		case '1':
			SwitchToDirectionalLight();
			break;

		case '2':
			SwitchToPointLight();
			break;

		case '3':
			SwitchToSpotLight();
			break;
		}
	}

private:
	bool CreateSceneGeometry() {
		auto& renderer = GetRenderer();

		// Create test cube
		m_testCube = Mesh::Mesh::CreateCube(&renderer, 2.0f);
		if (!m_testCube) {
			LOG_ERROR("Failed to create test cube");
			return false;
		}

		// Create ground plane
		m_groundPlane = Mesh::Mesh::CreatePlane(&renderer, 10.0f, 10.0f);
		if (!m_groundPlane) {
			LOG_WARNING("Failed to create ground plane, continuing without it");
		}

		return true;
	}

	bool CreateTextures() {
		auto& renderer = GetRenderer();
		auto device = renderer.GetDevice();

		if (!device) {
			LOG_ERROR("Device is null in CreateTextures");
			return false;
		}

		// Create checkerboard texture
		m_checkerTexture = std::make_shared<Renderer::Texture>();
		if (!m_checkerTexture->CreateCheckerboard(256, 256, device, 0xFFFFFFFF, 0xFF808080, 32)) {
			LOG_ERROR("Failed to create checkerboard texture");
			return false;
		}

		// Create solid color texture
		m_colorTexture = std::make_shared<Renderer::Texture>();
		if (!m_colorTexture->CreateSolidColor(256, 256, device, 0xFF4080FF)) {
			LOG_ERROR("Failed to create color texture");
			return false;
		}

		// Create sampler state
		m_samplerState = TEXTURE_MANAGER.CreateSamplerState(device);
		if (!m_samplerState) {
			LOG_ERROR("Failed to create sampler state");
			return false;
		}

		LOG_INFO("Successfully created textures");
		return true;
	}

	bool LoadShaders() {
		auto& renderer = GetRenderer();

		// Load vertex shader
		if (!renderer.LoadVertexShader(L"Shaders/VertexShader.hlsl", m_vertexShader, m_inputLayout,
			Mesh::VertexInputLayout, Mesh::VertexInputLayoutCount)) {
			// Try with .txt extension as fallback
			LOG_WARNING("Failed to load .hlsl shader, trying .hlsl.txt");
			// For now, create a simple constant buffer
			m_constantBuffer = renderer.CreateConstantBuffer(sizeof(Renderer::ConstantBuffer));
			return m_constantBuffer != nullptr;
		}

		// Load pixel shader - try simple one first
		if (!renderer.LoadPixelShader(L"Shaders/SimplePixelShader.hlsl.txt", m_pixelShader)) {
			LOG_WARNING("Failed to load SimplePixelShader.hlsl.txt, trying PixelShader.hlsl");
			if (!renderer.LoadPixelShader(L"Shaders/PixelShader.hlsl", m_pixelShader)) {
				LOG_WARNING("Failed to load pixel shader");
			}
		}

		// Create constant buffer
		m_constantBuffer = renderer.CreateConstantBuffer(sizeof(Renderer::ConstantBuffer));

		return m_constantBuffer != nullptr;
	}

	bool InitializeLights() {
		auto& renderer = GetRenderer();
		auto& lightManager = renderer.GetLightManager();

		// Create directional light (sun)
		m_directionalLight = std::make_shared<Renderer::DirectionalLight>();
		m_directionalLight->SetDirection(DirectX::XMFLOAT3(0.5f, -1.0f, 0.3f));
		m_directionalLight->SetColor(1.0f, 0.95f, 0.8f);
		m_directionalLight->SetIntensity(1.2f);
		m_directionalLight->SetCastShadows(true);
		lightManager.AddLight(m_directionalLight);

		// Create point light
		m_pointLight = std::make_shared<Renderer::PointLight>();
		m_pointLight->SetPosition(DirectX::XMFLOAT3(3.0f, 2.0f, 0.0f));
		m_pointLight->SetColor(1.0f, 0.2f, 0.2f);
		m_pointLight->SetIntensity(2.0f);
		m_pointLight->SetRange(8.0f);
		m_pointLight->SetCastShadows(true);
		m_pointLight->SetEnabled(false); // Start disabled
		lightManager.AddLight(m_pointLight);

		// Create spot light
		m_spotLight = std::make_shared<Renderer::SpotLight>();
		m_spotLight->SetPosition(DirectX::XMFLOAT3(-3.0f, 4.0f, -2.0f));
		m_spotLight->SetDirection(DirectX::XMFLOAT3(0.3f, -1.0f, 0.5f));
		m_spotLight->SetColor(0.2f, 0.2f, 1.0f);
		m_spotLight->SetIntensity(3.0f);
		m_spotLight->SetRange(10.0f);
		m_spotLight->SetInnerConeAngle(DirectX::XMConvertToRadians(20.0f));
		m_spotLight->SetOuterConeAngle(DirectX::XMConvertToRadians(35.0f));
		m_spotLight->SetCastShadows(true);
		m_spotLight->SetEnabled(false); // Start disabled
		lightManager.AddLight(m_spotLight);

		// Note: Ambient light would be set in the renderer's lighting system

		LOG_INFO("Lighting system initialized with " << lightManager.GetLightCount() << " lights");
		return true;
	}

	void UpdateLights(float deltaTime) {
		// Animate point light position
		static float pointLightAngle = 0.0f;
		pointLightAngle += deltaTime * 60.0f; // degrees per second
		if (pointLightAngle > 360.0f) pointLightAngle -= 360.0f;

		float radians = DirectX::XMConvertToRadians(pointLightAngle);
		float x = 3.0f * cosf(radians);
		float z = 3.0f * sinf(radians);
		m_pointLight->SetPosition(DirectX::XMFLOAT3(x, 2.0f, z));

		// Animate spot light direction
		static float spotAngle = 0.0f;
		spotAngle += deltaTime * 30.0f;
		if (spotAngle > 360.0f) spotAngle -= 360.0f;

		float spotRadians = DirectX::XMConvertToRadians(spotAngle);
		float dirX = sinf(spotRadians) * 0.5f;
		float dirZ = cosf(spotRadians) * 0.5f;
		m_spotLight->SetDirection(DirectX::XMFLOAT3(dirX, -1.0f, dirZ));
	}

	void RenderScene() {
		auto& renderer = GetRenderer();

		// Render main cube
		if (m_testCube) {
			Math::Matrix4 worldMatrix = Math::Matrix4::RotationY(DirectX::XMConvertToRadians(m_rotationY));
			renderer.UpdateConstantBuffer(worldMatrix, GetViewMatrix(), GetProjectionMatrix());
			m_testCube->Render(&renderer, worldMatrix);
		}

		// Render ground plane
		if (m_groundPlane) {
			Math::Matrix4 groundWorld = Math::Matrix4::Translation(0.0f, -1.5f, 0.0f);
			renderer.UpdateConstantBuffer(groundWorld, GetViewMatrix(), GetProjectionMatrix());
			m_groundPlane->Render(&renderer, groundWorld);
		}
	}

	void SwitchToDirectionalLight() {
		m_directionalLight->SetEnabled(true);
		m_pointLight->SetEnabled(false);
		m_spotLight->SetEnabled(false);
		LOG_INFO("Switched to Directional Light");
	}

	void SwitchToPointLight() {
		m_directionalLight->SetEnabled(false);
		m_pointLight->SetEnabled(true);
		m_spotLight->SetEnabled(false);
		LOG_INFO("Switched to Point Light");
	}

	void SwitchToSpotLight() {
		m_directionalLight->SetEnabled(false);
		m_pointLight->SetEnabled(false);
		m_spotLight->SetEnabled(true);
		LOG_INFO("Switched to Spot Light");
	}

	void ToggleShadows() {
		m_directionalLight->SetCastShadows(m_shadowsEnabled);
		m_pointLight->SetCastShadows(m_shadowsEnabled);
		m_spotLight->SetCastShadows(m_shadowsEnabled);
	}

	void LogDebugInfo() {
		LOG_INFO("=== DEBUG INFO ===");
		LOG_INFO("Rotation: " << m_rotationY);
		LOG_INFO("Current texture: " << m_currentTexture);
		LOG_INFO("Lighting enabled: " << (m_lightingEnabled ? "Yes" : "No"));
		LOG_INFO("Shadows enabled: " << (m_shadowsEnabled ? "Yes" : "No"));

		auto& lightManager = GetRenderer().GetLightManager();
		LOG_INFO("Active lights: " << lightManager.GetLightCount());
		LOG_INFO("Directional light: " << (m_directionalLight->IsEnabled() ? "On" : "Off"));
		LOG_INFO("Point light: " << (m_pointLight->IsEnabled() ? "On" : "Off"));
		LOG_INFO("Spot light: " << (m_spotLight->IsEnabled() ? "On" : "Off"));
		LOG_INFO("Camera position: " << GetCameraPosition().x << ", " << GetCameraPosition().y << ", " << GetCameraPosition().z);
	}

private:
	// Scene geometry
	std::shared_ptr<Mesh::Mesh> m_testCube;
	std::shared_ptr<Mesh::Mesh> m_groundPlane;
	float m_rotationY;

	// Lighting system
	std::shared_ptr<Renderer::DirectionalLight> m_directionalLight;
	std::shared_ptr<Renderer::PointLight> m_pointLight;
	std::shared_ptr<Renderer::SpotLight> m_spotLight;
	bool m_lightingEnabled;
	bool m_shadowsEnabled;

	// Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	// Textures
	std::shared_ptr<Renderer::Texture> m_checkerTexture;
	std::shared_ptr<Renderer::Texture> m_colorTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	int m_currentTexture;
};

// Console entry point for compatibility
int main() {
	return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}

// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Enable console for debugging
#ifdef _DEBUG
	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
	FILE* pCerr;
	freopen_s(&pCerr, "CONOUT$", "w", stderr);
	FILE* pCin;
	freopen_s(&pCin, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();
#endif

	TestGame game;

	if (!game.Initialize(hInstance, "DX11 Game Engine Test")) {
		MessageBoxW(nullptr, L"Failed to initialize game engine", L"Error", MB_ICONERROR);
		return -1;
	}

	game.Run();
	game.Shutdown();

#ifdef _DEBUG
	FreeConsole();
#endif

	return 0;
}