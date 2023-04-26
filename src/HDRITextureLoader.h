#pragma once
#include <string>
#include <DirectXMath.h>
#include "GfxObject.h"
#include "WinDef.h"

namespace DX = DirectX;

class HDRITextureLoader : public GfxObject
{
private:
	struct ConstantBuffer
	{
		DX::XMFLOAT4X4 mMatrix;
		DX::XMFLOAT4X4 vpMatrix;
	};

	com_ptr<ID3D11PixelShader> m_pHDRtoCubeMapPS;
	com_ptr<ID3D11VertexShader> m_pHDRtoCubeMapVS;
	com_ptr<ID3D11SamplerState> m_pHDRtoCubeMapSampler;
	com_ptr<ID3D11Buffer> m_pConstantBuffer;
	UINT m_hdrTextureSize;
	UINT m_irrTextureSize;
	com_ptr<ID3D11Texture2D> m_pHDRTexture512;
	com_ptr<ID3D11RenderTargetView> m_pHDRTexture512RTV;

	DX::XMMATRIX m_pMatrix;
	DX::XMMATRIX m_vMatrisis[6];
	DX::XMMATRIX m_mMatrises[6];

	void initPSO();
	void readHDRFile(std::string const& hdrFile, com_ptr<ID3D11Texture2D>& pHDRTexture);
	void renderEnvCubeMap(com_ptr<ID3D11Texture2D> const& pHDRTexture, com_ptr<ID3D11Texture2D>& pEnvCubeMap);
	void createCubeMap(com_ptr<ID3D11Texture2D>& pCubeMap, UINT cubeMapSize);
	void setViewPort(UINT width, UINT hight);

public:
	HDRITextureLoader(com_ptr<ID3D11Device> const& pDevice,
					com_ptr<ID3D11DeviceContext> const& pContext,
					com_ptr<ID3DUserDefinedAnnotation> const& pAnnotation);

	void loadEnvCubeMap(std::string const& hdrFile, com_ptr<ID3D11Texture2D>& pEnvCubeMap);

	~HDRITextureLoader() override;
};