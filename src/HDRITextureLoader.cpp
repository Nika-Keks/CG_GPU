#include "HDRITextureLoader.h"
#include <d3dcompiler.h>
#include "./ThirdParty/stb/stb_image.h"


void HDRITextureLoader::initPSO()
{
	com_ptr<ID3DBlob> pBlob;
	// pixel shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./", pBlob.GetAddressOf()));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pHDRtoCubeMapPS));

	// vertex shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./", pBlob.GetAddressOf()));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pHDRtoCubeMapVS));

	// sampler
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateSamplerState(&samplerDesc, &m_pHDRtoCubeMapSampler));

	// constat buffer
	ConstantBuffer cb;
	memset(&cb, 0, sizeof(cb));
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = { 0 };
	csd.pSysMem = &cb;
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateBuffer(&cbd, &csd, &m_pConstantBuffer));

	// cleate render target texture
	D3D11_TEXTURE2D_DESC hdrtd = {};
	hdrtd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrtd.Width = m_hdrTextureSize;
	hdrtd.Height = m_hdrTextureSize;
	hdrtd.BindFlags = D3D11_BIND_RENDER_TARGET;
	hdrtd.Usage = D3D11_USAGE_DEFAULT;
	hdrtd.CPUAccessFlags = 0;
	hdrtd.MiscFlags = 0;
	hdrtd.MipLevels = 1;
	hdrtd.ArraySize = 1;
	hdrtd.SampleDesc.Count = 1;
	hdrtd.SampleDesc.Quality = 0;
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateTexture2D(&hdrtd, nullptr, m_pHDRTexture512.GetAddressOf()));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateRenderTargetView(m_pHDRTexture512.Get(), nullptr, m_pHDRTexture512RTV.GetAddressOf()));
}

HDRITextureLoader::HDRITextureLoader(com_ptr<ID3D11Device> const& pDevice, com_ptr<ID3D11DeviceContext> const& pContext, com_ptr<ID3DUserDefinedAnnotation> const& pAnnotation)
	: GfxObject(pDevice, pContext, pAnnotation),
	m_hdrTextureSize(512),
	m_irrTextureSize(32)
{
	m_mMatrises[0] = DX::XMMatrixRotationY(DX::XM_PIDIV2);	// +X
	m_mMatrises[1] = DX::XMMatrixRotationY(-DX::XM_PIDIV2);	// -X
	m_mMatrises[2] = DX::XMMatrixRotationX(-DX::XM_PIDIV2);	// +Y
	m_mMatrises[3] = DX::XMMatrixRotationX(DX::XM_PIDIV2);	// -Y
	m_mMatrises[4] = DX::XMMatrixIdentity();				// +Z
	m_mMatrises[5] = DX::XMMatrixRotationY(DX::XM_PI);		// -Z

	m_vMatrix = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
	);
	const float nearp = 0.5f;
	const float farp = 1.5f;
	const float fov = DX::XM_PIDIV2;
	const float width = nearp / tanf(fov / 2.0f);
	const float height = width;
	m_pMatrix = DirectX::XMMatrixPerspectiveLH(2 * width, 2 * height, nearp, farp);
}

void HDRITextureLoader::loadEnvCubeMap(std::string const& hdrFile, com_ptr<ID3D11Texture2D>& pEnvCubeMap, com_ptr<ID3D11ShaderResourceView>& pEnvCubeMapSRV)
{
	// read hdr file to texture & create srv
	com_ptr<ID3D11Texture2D> pHDRTexture;
	readHDRFile(hdrFile, pHDRTexture);

	// render cube map from hdr texture
	renderEnvCubeMap(pHDRTexture, pEnvCubeMap);
}

void HDRITextureLoader::renderEnvCubeMap(com_ptr<ID3D11Texture2D> const& pHDRTexture, com_ptr<ID3D11Texture2D>& pEnvCubeMap)
{
	// create shder resurce view
	com_ptr<ID3D11ShaderResourceView> pHDRTextureSRV;
	m_pDevice->CreateShaderResourceView(pHDRTexture.Get(), nullptr, &pHDRTextureSRV);

	// create cube map
	createCubeMap(pEnvCubeMap, m_hdrTextureSize);

	startEvent(L"HDRtoEnv");

	m_pContext->ClearState();
	m_pContext->OMSetRenderTargets(1, m_pHDRTexture512RTV.GetAddressOf(), nullptr);

	// set view port & scissors rect
	setViewPort(m_hdrTextureSize, m_hdrTextureSize);

	m_pContext->IASetInputLayout(nullptr);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pContext->VSSetShader(m_pHDRtoCubeMapVS.Get(), nullptr, 0);
	m_pContext->PSSetShader(m_pHDRtoCubeMapPS.Get(), nullptr, 0);
	m_pContext->PSSetShaderResources(0, 1, pHDRTextureSRV.GetAddressOf());
	m_pContext->PSSetSamplers(0, 1, m_pHDRtoCubeMapSampler.GetAddressOf());

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ConstantBuffer cb = {};

	for (UINT i = 0; i < 6; ++i)
	{
		m_pContext->ClearRenderTargetView(m_pHDRTexture512RTV.Get(), clearColor);
		cb.mMatrix = m_mMatrises[i];
		cb.vpMatrix = (m_mMatrises[i] * m_vMatrix) * m_pMatrix;
		m_pContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		m_pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pContext->Draw(4, 0);
		m_pContext->CopySubresourceRegion(pEnvCubeMap.Get(), i, 0, 0, 0, m_pHDRTexture512.Get(), 0, nullptr);
	}
	endEvent();
}

void HDRITextureLoader::setViewPort(UINT width, UINT hight)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)width;
	viewport.Height = (float)hight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D11_RECT rect = {};
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = hight;

	m_pContext->RSSetViewports(1, &viewport);
	m_pContext->RSSetScissorRects(1, &rect);
}


void HDRITextureLoader::createCubeMap(com_ptr<ID3D11Texture2D>& pCubeMap, UINT cubeMapSize)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Width = cubeMapSize;
	desc.Height = cubeMapSize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateTexture2D(&desc, nullptr, &pCubeMap));
}

void HDRITextureLoader::readHDRFile(std::string const& hdrFile, com_ptr<ID3D11Texture2D>& pHDRTexture)
{
	// read hdr file
	int h = 0, w = 0, c = 0;
	auto imgData = stbi_loadf(hdrFile.c_str(), &w, &h, &c, 4);

	// crweate texture
	D3D11_TEXTURE2D_DESC hdrtd;
	hdrtd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrtd.Width = w;
	hdrtd.Height = h;
	hdrtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	hdrtd.Usage = D3D11_USAGE_DEFAULT;
	hdrtd.CPUAccessFlags = 0;
	hdrtd.MiscFlags = 0;
	hdrtd.MipLevels = 1;
	hdrtd.ArraySize = 1;
	hdrtd.SampleDesc.Count = 1;
	hdrtd.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA hdrtdata = {};
	hdrtdata.pSysMem = imgData;
	hdrtdata.SysMemPitch = 4u * w * sizeof(float);
	hdrtdata.SysMemSlicePitch = 0;

	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateTexture2D(&hdrtd, &hdrtdata, &pHDRTexture));
}


HDRITextureLoader::~HDRITextureLoader() {}
