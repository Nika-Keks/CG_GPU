#include "HDRITextureLoader.h"
#include <d3dcompiler.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./ThirdParty/stb/stb_image.h"


void HDRITextureLoader::initPSO()
{
	com_ptr<ID3DBlob> pBlob;
	// pixel hdr shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./HDRtoEnvPixelShader.cso", &pBlob));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pHDRtoCubeMapPS));

	// pixel irr shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./IRRPixelShader.cso", &pBlob));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pIrrCubeMapPS));
	
	// pixel prefiltered shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./PrefilteredPixelShader.cso", &pBlob));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPrefCubeMapPS));
	
	// pixel prefiltered shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./PreintegratedBRDFPixelShader.cso", &pBlob));
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPtreintBRDFPS));

	// vertex shader
	THROW_IF_FAILED(GtxObjError, D3DReadFileToBlob(L"./HDRtoEnvVertexShader.cso", &pBlob));
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

	// constat buffer VS
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBufferVS);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateBuffer(&cbd, nullptr, &m_pConstantBufferVS));
	
	// constat buffer PS
	cbd.ByteWidth = sizeof(ConstantBufferPS);
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateBuffer(&cbd, nullptr, &m_pConstantBufferPS));


	// cleate render target texture
	createTextureRTV(m_hdrTextureSize, m_pHDRTexture, m_pHDRTextureRTV, DXGI_FORMAT_R32G32B32A32_FLOAT);
	createTextureRTV(m_irrTextureSize, m_pIrrTexture, m_pIrrTextureRTV, DXGI_FORMAT_R32G32B32A32_FLOAT);
	createTextureRTV(m_prefTextureSize, m_pPrefTexture, m_pPrefTextureRTV, DXGI_FORMAT_R32G32B32A32_FLOAT);
	createTextureRTV(m_preintegratedBRDFSize, m_pPreintBRDFTexture, m_pPreintBRDFTextureRTV, DXGI_FORMAT_R32G32_FLOAT);

}

void HDRITextureLoader::createTextureRTV(UINT size, com_ptr<ID3D11Texture2D>& pTexture, com_ptr<ID3D11RenderTargetView>& pTextureRTV, DXGI_FORMAT format)
{
	D3D11_TEXTURE2D_DESC hdrtd = {};
	hdrtd.Format = format;
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
	m_irrTextureSize(32),
	m_prefTextureSize(128),
	m_mipLevels(4),
	m_preintegratedBRDFSize(128)
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

	// render irradiance map from env cube map
	renderIrrCubeMap(pEnvCubeMap, pIrrCubeMap);

	// render prefiltered color
	com_ptr<ID3D11Texture2D> pPrefCubeMap;
	renderPreliteredCubeMap(pEnvCubeMap, pPrefCubeMap);

	// rebder preintegrated BRDF
	com_ptr<ID3D11Texture2D> pPreintBRDFTexture;
	renderPreintBRDFTexture(pPreintBRDFTexture);
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
	ConstantBufferVS cb = {};

	for (UINT i = 0; i < 6; ++i)
	{
		m_pContext->ClearRenderTargetView(m_pHDRTextureRTV.Get(), clearColor);
		DX::XMStoreFloat4x4(&cb.mMatrix, DX::XMMatrixTranspose(m_mMatrises[i]));
		DX::XMStoreFloat4x4(&cb.vpMatrix, DX::XMMatrixTranspose(m_vMatrisis[i] * m_pMatrix));
		m_pContext->UpdateSubresource(m_pConstantBufferVS.Get(), 0, nullptr, &cb, 0, 0);
		m_pContext->VSSetConstantBuffers(0, 1, m_pConstantBufferVS.GetAddressOf());
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
	ConstantBufferVS cb = {};

	for (UINT i = 0; i < 6; ++i)
	{
		m_pContext->ClearRenderTargetView(m_pIrrTextureRTV.Get(), clearColor);
		DX::XMStoreFloat4x4(&cb.mMatrix, DX::XMMatrixTranspose(m_mMatrises[i]));
		DX::XMStoreFloat4x4(&cb.vpMatrix, DX::XMMatrixTranspose(m_vMatrisis[i] * m_pMatrix));
		m_pContext->UpdateSubresource(m_pConstantBufferVS.Get(), 0, nullptr, &cb, 0, 0);
		m_pContext->VSSetConstantBuffers(0, 1, m_pConstantBufferVS.GetAddressOf());
		m_pContext->Draw(4, 0);
		m_pContext->CopySubresourceRegion(pIrrCubeMap.Get(), i, 0, 0, 0, m_pIrrTexture.Get(), 0, nullptr);
	}

	endEvent();
}

void HDRITextureLoader::renderPreliteredCubeMap(com_ptr<ID3D11Texture2D> const& pEnvCubeMap, com_ptr<ID3D11Texture2D>& pPrefCubeMap)
{
	// create shder resurce view
	com_ptr<ID3D11ShaderResourceView> pEnvCubeMapSRV;
	m_pDevice->CreateShaderResourceView(pEnvCubeMap.Get(), nullptr, &pEnvCubeMapSRV);

	// create cube map
	createCubeMap(pPrefCubeMap, m_prefTextureSize);

	startEvent(L"PrefilteredColor");

	m_pContext->ClearState();
	m_pContext->OMSetRenderTargets(1, m_pPrefTextureRTV.GetAddressOf(), nullptr);

	// set view port & scissors rect
	setViewPort(m_prefTextureSize, m_prefTextureSize);

	m_pContext->IASetInputLayout(nullptr);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pContext->VSSetShader(m_pHDRtoCubeMapVS.Get(), nullptr, 0);
	m_pContext->PSSetShader(m_pPrefCubeMapPS.Get(), nullptr, 0);
	m_pContext->PSSetShaderResources(0, 1, pEnvCubeMapSRV.GetAddressOf());
	m_pContext->PSSetSamplers(0, 1, m_pHDRtoCubeMapSampler.GetAddressOf());

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ConstantBufferVS vcb = {};
	ConstantBufferPS pcb = {};
	D3D11_BOX mipBox = {};
	mipBox.left = mipBox.top = mipBox.front = 0;
	mipBox.back = 1;
	for (UINT i = 0; i < 6; ++i)
	{
		DX::XMStoreFloat4x4(&vcb.mMatrix, DX::XMMatrixTranspose(m_mMatrises[i]));
		DX::XMStoreFloat4x4(&vcb.vpMatrix, DX::XMMatrixTranspose(m_vMatrisis[i] * m_pMatrix));
		m_pContext->UpdateSubresource(m_pConstantBufferVS.Get(), 0, nullptr, &vcb, 0, 0);
		m_pContext->VSSetConstantBuffers(0, 1, m_pConstantBufferVS.GetAddressOf());
		UINT mipLevelSize = m_prefTextureSize;
		for (UINT j = 0; j < m_mipLevels; ++j)
		{
			m_pContext->ClearRenderTargetView(m_pPrefTextureRTV.Get(), clearColor);
			
			// set roughness
			pcb.roughness = (float)j / (float)(m_mipLevels - 1);
			m_pContext->UpdateSubresource(m_pConstantBufferPS.Get(), 0, nullptr, &pcb, 0, 0);
			m_pContext->PSSetConstantBuffers(0, 1, m_pConstantBufferPS.GetAddressOf());

			setViewPort(mipLevelSize, mipLevelSize);
			mipBox.right = mipBox.bottom = mipLevelSize;

			m_pContext->Draw(4, 0);
			m_pContext->CopySubresourceRegion(pPrefCubeMap.Get(), D3D11CalcSubresource(j, i, m_mipLevels), 0, 0, 0, m_pPrefTexture.Get(), 0, &mipBox);
			mipLevelSize /= 2;
		}
	}
	endEvent();
}

void HDRITextureLoader::renderPreintBRDFTexture(com_ptr<ID3D11Texture2D>& pPreintBRDFTexture)
{
	// create texture
	D3D11_TEXTURE2D_DESC txd = {};
	txd.Format = DXGI_FORMAT_R32G32_FLOAT;
	txd.Width = m_prefTextureSize;
	txd.Height = m_prefTextureSize;
	txd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	txd.Usage = D3D11_USAGE_DEFAULT;
	txd.CPUAccessFlags = 0;
	txd.MiscFlags = 0;
	txd.MipLevels = 1;
	txd.ArraySize = 1;
	txd.SampleDesc.Count = 1;
	txd.SampleDesc.Quality = 0;
	THROW_IF_FAILED(GtxObjError, m_pDevice->CreateTexture2D(&txd, nullptr, &pPreintBRDFTexture));

	// setup pipeline
	startEvent(L"Ptreintegrated BRDF");

	m_pContext->ClearState();
	m_pContext->OMSetRenderTargets(1, m_pPreintBRDFTextureRTV.GetAddressOf(), nullptr);

	// set view port & scissors rect
	setViewPort(m_preintegratedBRDFSize, m_preintegratedBRDFSize);

	m_pContext->IASetInputLayout(nullptr);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pContext->VSSetShader(m_pHDRtoCubeMapVS.Get(), nullptr, 0);
	m_pContext->PSSetShader(m_pPtreintBRDFPS.Get(), nullptr, 0);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ConstantBufferVS vcb = {};
	UINT const defaultIndex = 4;
	DX::XMStoreFloat4x4(&vcb.mMatrix, DX::XMMatrixTranspose(m_mMatrises[defaultIndex]));
	DX::XMStoreFloat4x4(&vcb.vpMatrix, DX::XMMatrixTranspose(m_vMatrisis[defaultIndex] * m_pMatrix));
	m_pContext->UpdateSubresource(m_pConstantBufferVS.Get(), 0, nullptr, &vcb, 0, 0);
	m_pContext->VSSetConstantBuffers(0, 1, m_pConstantBufferVS.GetAddressOf());
	m_pContext->ClearRenderTargetView(m_pPreintBRDFTextureRTV.Get(), clearColor);
	m_pContext->Draw(4, 0);
	m_pContext->CopySubresourceRegion(pPreintBRDFTexture.Get(), 0, 0, 0, 0, m_pPreintBRDFTexture.Get(), 0, nullptr);

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

void HDRITextureLoader::createCubeMap(com_ptr<ID3D11Texture2D>& pCubeMap, UINT cubeMapSize, UINT mipLevels)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Width = cubeMapSize;
	desc.Height = cubeMapSize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	desc.MipLevels = mipLevels;
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
