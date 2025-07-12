#include <Windows.h>
#include <memory>
#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "Renderer/D3D11Renderer.h"
#include "Renderer/Texture.h"

using namespace GameEngine;

class TestGame : public Core::Engine {
public:
    TestGame() : m_rotationY(0.0f), m_currentTexture(0) {}

protected:
    bool OnInitialize() override {
        LOG_INFO("TestGame initializing...");

        // Create test cube
        m_testCube = Mesh::Mesh::CreateCube(&GetRenderer(), 2.0f);
        if (!m_testCube) {
            LOG_ERROR("Failed to create test cube");
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

        // Set up camera
        SetCameraPosition(Math::Vector3(0.0f, 0.0f, -5.0f));
        SetCameraTarget(Math::Vector3(0.0f, 0.0f, 0.0f));

        LOG_INFO("TestGame initialized successfully");
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Rotate the cube
        m_rotationY += deltaTime * 45.0f; // 45 degrees per second
        if (m_rotationY > 360.0f) {
            m_rotationY -= 360.0f;
        }

        // Handle additional input
        static bool spacePressed = false;
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            if (!spacePressed) {
                LOG_INFO("Space pressed - Current rotation: " << m_rotationY);
                spacePressed = true;
            }
        } else {
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
        } else if (m_currentTexture == 1 && m_colorTexture) {
            renderer.SetTexture(m_colorTexture->GetSRV(), 0);
        }

        if (m_samplerState) {
            renderer.SetSampler(m_samplerState.Get(), 0);
        }

        // Create world matrix with rotation
        Math::Matrix4 worldMatrix = Math::Matrix4::RotationY(DirectX::XMConvertToRadians(m_rotationY));

        // Update constant buffer
        renderer.UpdateConstantBuffer(worldMatrix, GetViewMatrix(), GetProjectionMatrix());

        // Render the cube
        if (m_testCube) {
            m_testCube->Render(&renderer, worldMatrix);
        }
    }

    void OnShutdown() override {
        LOG_INFO("TestGame shutting down...");

        m_testCube.reset();
        m_vertexShader.Reset();
        m_pixelShader.Reset();
        m_inputLayout.Reset();
        m_constantBuffer.Reset();

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
        }
    }

private:
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

private:
    std::shared_ptr<Mesh::Mesh> m_testCube;
    float m_rotationY;

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

    if (!game.Initialize(hInstance, "DX11 Game Engine Test", 1024, 768)) {
        MessageBox(nullptr, L"Failed to initialize game engine", L"Error", MB_ICONERROR);
        return -1;
    }

    game.Run();
    game.Shutdown();

#ifdef _DEBUG
    FreeConsole();
#endif

    return 0;
}
