#include "HDRITextureLoader.h"
#include <d3dcompiler.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./ThirdParty/stb/stb_image.h"


void HDRITextureLoader::initPSO()
{
	com_ptr<ID3DBlob> pBlob;
	// pixel hdr shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./HDRtoEnvPixelShader.cso", pBlob.GetAddressOf()));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pHDRtoCubeMapPS));

	// pixel irr shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./IRRPixelShader.cso", pBlob.GetAddressOf()));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pIrrCubeMapPS));

	// vertex shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./HDRtoEnvVertexShader.cso", pBlob.GetAddressOf()));
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
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;

	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateBuffer(&cbd, nullptr, &m_pConstantBuffer));

	// cleate render target texture
	createTextureRTV(m_hdrTextureSize, m_pHDRTexture, m_pHDRTextureRTV);
	createTextureRTV(m_irrTextureSize, m_pIrrTexture, m_pIrrTextureRTV);
}

void HDRITextureLoader::createTextureRTV(UINT size, com_ptr<ID3D11Texture2D>& pTexture, com_ptr<ID3D11RenderTargetView>& pTextureRTV)
{
	D3D11_TEXTURE2D_DESC hdrtd = {};
	hdrtd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrtd.Width = size;
	hdrtd.Height = size;
	hdrtd.BindFlags = D3D11_BIND_RENDER_TARGET;
	hdrtd.Usage = D3D11_USAGE_DEFAULT;
	hdrtd.CPUAccessFlags = 0;
	hdrtd.MiscFlags = 0;
	hdrtd.MipLevels = 1;
	hdrtd.ArraySize = 1;
	hdrtd.SampleDesc.Count = 1;
	hdrtd.SampleDesc.Quality = 0;
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateTexture2D(&hdrtd, nullptr, pTexture.GetAddressOf()));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateRenderTargetView(pTexture.Get(), nullptr, pTextureRTV.GetAddressOf()));
}

HDRITextureLoader::HDRITextureLoader(com_ptr<ID3D11Device> const& pDevice, com_ptr<ID3D11DeviceContext> const& pContext, com_ptr<ID3DUserDefinedAnnotation> const& pAnnotation)
	: GfxObject(pDevice, pContext, pAnnotation),
	m_hdrTextureSize(1024),
	m_irrTextureSize(32)
{
	initPSO();
	m_mMatrises[0] = DX::XMMatrixRotationY(DX::XM_PIDIV2);	// +X
	m_mMatrises[1] = DX::XMMatrixRotationY(-DX::XM_PIDIV2);	// -X
	m_mMatrises[2] = DX::XMMatrixRotationX(-DX::XM_PIDIV2);	// +Y
	m_mMatrises[3] = DX::XMMatrixRotationX(DX::XM_PIDIV2);	// -Y
	m_mMatrises[4] = DX::XMMatrixIdentity();				// +Z
	m_mMatrises[5] = DX::XMMatrixRotationY(DX::XM_PI);		// -Z

	//m_vMatrix = DirectX::XMMatrixLookToLH(
	//	{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
	//);
	m_vMatrisis[0] = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
	);	// +X
	m_vMatrisis[1] = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
	);	// -X
	m_vMatrisis[2] = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }
	);	// +Y
	m_vMatrisis[3] = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }
	);	// -Y
	m_vMatrisis[4] = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
	);	// +Z
	m_vMatrisis[5] = DirectX::XMMatrixLookToLH(
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
	);	// -Z
	const float nearp = 0.5f;
	const float farp = 1.5f;
	const float fov = DX::XM_PIDIV2;
	const float width = nearp / tanf(fov / 2.0f);
	const float height = width;
	m_pMatrix = DirectX::XMMatrixPerspectiveLH(2 * width, 2 * height, nearp, farp);
}


void HDRITextureLoader::loadEnvCubeMap(std::string const& hdrFile, com_ptr<ID3D11Texture2D>& pEnvCubeMap, com_ptr<ID3D11Texture2D>& pIrrCubeMap)
{
	// read hdr file to texture & create srv
	com_ptr<ID3D11Texture2D> pHDRTexture;
	readHDRFile(hdrFile, pHDRTexture);

	// render cube map from hdr texture
	renderEnvCubeMap(pHDRTexture, pEnvCubeMap);

	//render irradiance map from env cube map
	renderIrrCubeMap(pEnvCubeMap, pIrrCubeMap);
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
	m_pContext->OMSetRenderTargets(1, m_pHDRTextureRTV.GetAddressOf(), nullptr);
	
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
		m_pContext->ClearRenderTargetView(m_pHDRTextureRTV.Get(), clearColor);
		DX::XMStoreFloat4x4(&cb.mMatrix, DX::XMMatrixTranspose(m_mMatrises[i]));
		DX::XMStoreFloat4x4(&cb.vpMatrix, DX::XMMatrixTranspose(m_vMatrisis[i] * m_pMatrix));
		m_pContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		m_pContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
		m_pContext->Draw(4, 0);
		m_pContext->CopySubresourceRegion(pEnvCubeMap.Get(), i, 0, 0, 0, m_pHDRTexture.Get(), 0, nullptr);
	}
	endEvent();
}

void HDRITextureLoader::renderIrrCubeMap(com_ptr<ID3D11Texture2D> const& pEnvCubeMap, com_ptr<ID3D11Texture2D>& pIrrCubeMap)
{
	// create shder resurce view
	com_ptr<ID3D11ShaderResourceView> pEnvCubeMapSRV;
	m_pDevice->CreateShaderResourceView(pEnvCubeMap.Get(), nullptr, &pEnvCubeMapSRV);

	// create cube map
	createCubeMap(pIrrCubeMap, m_irrTextureSize);

	startEvent(L"ENVtoIrradiance");

	m_pContext->ClearState();
	m_pContext->OMSetRenderTargets(1, m_pIrrTextureRTV.GetAddressOf(), nullptr);

	// set view port & scissors rect
	setViewPort(m_irrTextureSize, m_irrTextureSize);

	m_pContext->IASetInputLayout(nullptr);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pContext->VSSetShader(m_pHDRtoCubeMapVS.Get(), nullptr, 0);
	m_pContext->PSSetShader(m_pIrrCubeMapPS.Get(), nullptr, 0);
	m_pContext->PSSetShaderResources(0, 1, pEnvCubeMapSRV.GetAddressOf());
	m_pContext->PSSetSamplers(0, 1, m_pHDRtoCubeMapSampler.GetAddressOf());

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ConstantBuffer cb = {};

	for (UINT i = 0; i < 6; ++i)
	{
		m_pContext->ClearRenderTargetView(m_pIrrTextureRTV.Get(), clearColor);
		DX::XMStoreFloat4x4(&cb.mMatrix, DX::XMMatrixTranspose(m_mMatrises[i]));
		DX::XMStoreFloat4x4(&cb.vpMatrix, DX::XMMatrixTranspose(m_vMatrisis[i] * m_pMatrix));
		m_pContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		m_pContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
		m_pContext->Draw(4, 0);
		m_pContext->CopySubresourceRegion(pIrrCubeMap.Get(), i, 0, 0, 0, m_pIrrTexture.Get(), 0, nullptr);
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
