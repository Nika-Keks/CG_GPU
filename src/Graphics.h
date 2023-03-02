#pragma once
#include "WinDef.h"
#include <d3d11_1.h>
#include "BaseException.h"
#include <DirectXMath.h>
#include <vector>


class Graphics
{
	class GtxError : public BaseException
	{
	public:
		GtxError(int line, const char* file);
		const char* GetType() const noexcept override;
	};
public:
	Graphics( HWND hWnd );
	Graphics( const Graphics& ) = delete;
	Graphics& operator=( const Graphics& ) = delete;
	~Graphics();
	void chSwapChain(int height, int width);
	void EndFrame();
	void ClearBuffer( float red,float green,float blue ) noexcept;
	void DrawTest(float angle, float x, float y);
private:

	struct RenderTargetTexture
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture2D;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
		D3D11_VIEWPORT viewport;
	};

	struct Vertex
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
	};

	struct ProcessTextureVertex 
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
		struct
		{
			float x;
			float y;
		} texcoord;
	};

	struct VSConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};

	struct HDRConstantBuffer
	{
		float averageLumen;
	};

	ProcessTextureVertex m_quad_vertices[4] =
	{
		{-1.f,-1.f,0.f, 0,0,0,1, 0.f, 1.f},
		{ 1.f,-1.f,0.f, 0,0,0,1, 1.f, 1.f},
		{-1.f, 1.f,0.f, 0,0,0,1, 0.f, 0.f},
		{ 1.f, 1.f,0.f, 0,0,0,1, 1.f, 0.f},
	};

	const unsigned short m_quad_indices[6] = { 0,3,1,3,0,2 };

	struct Geometry
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		int indicesSize;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	};

	void startEvent(LPCWSTR);
	void endEvent();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> createPixelShader(const wchar_t* psPath);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> createVertexShader(const wchar_t* vsPath, ID3DBlob** vertexBlob);
	RenderTargetTexture CreateTexture(int height, int width);
	void downsampleTexture(const RenderTargetTexture& inputTex, const RenderTargetTexture& resultTex);
	void updateRenderTargets(int height, int width);
	Geometry createQuad(int height, int width);

	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
	Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_pAnnotation;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pSimpleVSBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pCopyVSBlob;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState; 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pExposureSampler;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pProcessTextureLayout;

	RenderTargetTexture m_sceneRenderTarget;
	RenderTargetTexture m_postprocessedRenderTarget;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_averageLumenCPUTexture;
	
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VSSimple;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSSimple;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VSCopy;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSCopy;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSBrightness;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSHdr;

	// HDR
	std::vector<RenderTargetTexture> m_scaledHDRTargets;
	float m_prevExposure = 0;
	
	struct
	{
		int widht;
		int height;
	}bufferSize;
};
