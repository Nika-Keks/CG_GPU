#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <DirectXMath.h>

namespace DX = DirectX;
class PixelShader
{
public:
	PixelShader(const wchar_t* psPath, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		THROW_IF_FAILED(BaseException, D3DReadFileToBlob(psPath, &pBlob));
		THROW_IF_FAILED(BaseException, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pixelShader));
	}
	template<typename T>
	void CreateConstantBuffer(UINT idx, T* pSysMem)
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> cb;
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(T);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = pSysMem;
		sd.SysMemPitch = 0;
		sd.SysMemSlicePitch = 0;
		THROW_IF_FAILED(BaseException, m_pDevice->CreateBuffer(&desc, &sd, &cb));
		m_constantBuffers[idx] = cb;
	}
	void Set()
	{
		m_pContext->PSSetShader(m_pixelShader.Get(), nullptr, 0u);
		SetConstantBuffers();
	}
	void SetConstantBuffers()
	{
		for (auto bufferPair = m_constantBuffers.begin(); bufferPair != m_constantBuffers.end(); bufferPair++)
		{
			auto& idx = bufferPair->first;
			auto& buffer = bufferPair->second;
			m_pContext->PSSetConstantBuffers(idx, 1, buffer.GetAddressOf());
		}
	}
protected:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	std::map<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_constantBuffers;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& m_pContext;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
};

__declspec(align(16))
struct PBRParams
{
	DX::XMFLOAT3 albedo;
	float roughness;
	float metalness;
};

class PBRPixelShader : public PixelShader
{
		
public:
	PBRPixelShader(const wchar_t* psPath, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, 
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) : PixelShader(psPath, pDevice, pContext)
	{
		PixelShader::CreateConstantBuffer(1, &m_params);
		PixelShader::SetConstantBuffers();
	};
private:
	PBRParams m_params;
};