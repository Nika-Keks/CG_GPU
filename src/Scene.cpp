#include "Scene.h"
#include <d3dcompiler.h>


void Scene::clear()
{
	m_objects = {};
}

void Scene::update(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext)
{
	if (m_pVertexShader == nullptr || m_pInputLayout == nullptr || m_pEnvSphereInputLayout == nullptr || m_pEnvSphereVertexShader == nullptr)
		initResourses(pDevice, pContext);
	else
	{
		pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
		pContext->IASetInputLayout(m_pInputLayout.Get());
	}
}

void Scene::render(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext)
{
	if (m_pVertexShader == nullptr || m_pInputLayout == nullptr || m_pEnvSphereInputLayout == nullptr || m_pEnvSphereVertexShader == nullptr)
		initResourses(pDevice, pContext);

	if (m_environmentSphere)
	{
		startEvent(L"SceneEnv");
		pContext->VSSetShader(m_pEnvSphereVertexShader.Get(), nullptr, 0u);
		pContext->IASetInputLayout(m_pEnvSphereInputLayout.Get());
		pContext->PSSetSamplers(1, 1, m_pBRDFSampler.GetAddressOf());
		m_environmentSphere->render(pDevice, pContext);
		pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
		pContext->IASetInputLayout(m_pInputLayout.Get());
		ID3D11ShaderResourceView* srvList[] = { m_pIrrCubeMapSRV.Get(), m_pPrefTextureSRV.Get(), m_pPreintBRDFTextureSRV.Get() };
		pContext->PSSetShaderResources(0, 3, srvList);

		endEvent();
	}

	startEvent(L"SceneObjs");
	for (auto& obj : m_objects)
	{
		obj->render(pDevice, pContext);
	}
	endEvent();
	startEvent(L"ScenePhObjs");
	for (auto& obj : m_physicallObjects)
	{
		obj->render(pDevice, pContext);
	}
	endEvent();
}

void Scene::setEnvSphere(float radius, wchar_t const* texturePath, DirectX::XMVECTOR pos, Camera const& camera)
{
	m_environmentSphere = std::make_shared<EnvSphere>(pos, radius, camera, m_pEnvCubeMapSRV);
}

void Scene::initResourses(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	THROW_IF_FAILED(DrawError,
		pContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation),
			reinterpret_cast<void**>(m_pAnnotation.GetAddressOf()))
	);
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	// input (vertex) layout (2d position only)
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Normal",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	if (m_pVertexShader == nullptr)
	{
		THROW_IF_FAILED(DrawError, D3DReadFileToBlob(m_vsPath, &pBlob));
		THROW_IF_FAILED(DrawError, pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader));
		THROW_IF_FAILED(DrawError, pDevice->CreateInputLayout(
			ied, (UINT)std::size(ied),
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			&m_pInputLayout
		));
	}
	if (m_pEnvSphereVertexShader == nullptr)
	{
		THROW_IF_FAILED(DrawError, D3DReadFileToBlob(m_vsEnvSpherePath, &pBlob));
		THROW_IF_FAILED(DrawError, pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pEnvSphereVertexShader));
		THROW_IF_FAILED(DrawError, pDevice->CreateInputLayout(
			ied, (UINT)std::size(ied),
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			&m_pEnvSphereInputLayout
		));
	}

	if (!m_pHDRLoader)
		m_pHDRLoader = std::make_shared<HDRITextureLoader>(pDevice, pContext, m_pAnnotation);
	if (!m_pEnvCubeMap)
	{
		m_pHDRLoader->loadEnvCubeMap("./../../image/misty_farm_road_4k.hdr", m_pEnvCubeMap, m_pIrrCubeMap, m_pPrefTexture, m_pPreintBRDFTexture);
		THROW_IF_FAILED(DrawError, pDevice->CreateShaderResourceView(m_pEnvCubeMap.Get(), nullptr, m_pEnvCubeMapSRV.GetAddressOf()));
		THROW_IF_FAILED(DrawError, pDevice->CreateShaderResourceView(m_pIrrCubeMap.Get(), nullptr, m_pIrrCubeMapSRV.GetAddressOf()));
		THROW_IF_FAILED(DrawError, pDevice->CreateShaderResourceView(m_pPrefTexture.Get(), nullptr, m_pPrefTextureSRV.GetAddressOf()));
		THROW_IF_FAILED(DrawError, pDevice->CreateShaderResourceView(m_pPreintBRDFTexture.Get(), nullptr, m_pPreintBRDFTextureSRV.GetAddressOf()));
		if (m_environmentSphere)
			m_environmentSphere->resetEndCubeMapSRV(m_pEnvCubeMapSRV);
	}
	if (!m_pBRDFSampler) 
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		THROW_IF_FAILED(DrawError, pDevice->CreateSamplerState(&samplerDesc, &m_pBRDFSampler));
	}
	update(pDevice, pContext);
}

void Scene::setPBRParams(UINT idx, PBRParams params)
{
	assert(idx < m_physicallObjects.size());
	m_physicallObjects[idx]->setPBRParams(params);
}

unsigned Scene::phisicallyDrawableSize() const
{
	return static_cast<unsigned>(m_physicallObjects.size());
}

void Scene::startEvent(LPCWSTR eventName)
{
#ifdef _DEBUG
	m_pAnnotation->BeginEvent(eventName);
#endif
}


void Scene::endEvent()
{
#ifdef _DEBUG
	m_pAnnotation->EndEvent();
#endif
}