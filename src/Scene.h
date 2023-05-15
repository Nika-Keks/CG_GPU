#pragma once
#include <memory>
#include <vector>
#include "PhysicallyDrawable.h"
#include "Camera.h"
#include "HDRITextureLoader.h"
#include "EnvSphere.h"


class Scene : public Drawable
{
public:
	template <typename DrawT, typename... argsT>
	void addDrawable(argsT... params);
	template <typename DrawT, typename... argsT>
	void addPhysicallyDrawable(argsT... params);
	void setEnvSphere(float radius, wchar_t const* texturePath, DirectX::XMVECTOR pos, Camera const& camera);
	unsigned phisicallyDrawableSize() const;
	void startEvent(LPCWSTR eventName);
	void endEvent();
	void setPBRParams(UINT idx, PBRParams params);
	//void addLight();
	void clear();
	void update(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext) override;

private:

	void initResourses(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr < ID3D11DeviceContext> const& pContext);

	std::vector<std::shared_ptr<Drawable>> m_objects;
	std::vector<std::shared_ptr<PhysicallyDrawable>> m_physicallObjects;
	std::shared_ptr<EnvSphere> m_environmentSphere;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pEnvSphereVertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pEnvSphereInputLayout;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
	Microsoft::WRL::ComPtr< ID3DUserDefinedAnnotation> m_pAnnotation;
	static constexpr wchar_t const * const m_vsPath = L"VertexShader.cso";
	static constexpr wchar_t const* const m_vsEnvSpherePath = L"EnvSphereVertexShader.cso";

	std::shared_ptr<HDRITextureLoader> m_pHDRLoader;
	com_ptr<ID3D11Texture2D> m_pEnvCubeMap;
	com_ptr<ID3D11ShaderResourceView> m_pEnvCubeMapSRV;
	com_ptr<ID3D11Texture2D> m_pIrrCubeMap;
	com_ptr<ID3D11ShaderResourceView> m_pIrrCubeMapSRV;
	com_ptr<ID3D11Texture2D> m_pPrefTexture;
	com_ptr<ID3D11ShaderResourceView> m_pPrefTextureSRV;
	com_ptr<ID3D11Texture2D> m_pPreintBRDFTexture;
	com_ptr<ID3D11ShaderResourceView> m_pPreintBRDFTextureSRV;
	com_ptr<ID3D11SamplerState> m_pBRDFSampler;
};

template <typename DrawT, typename... argsT>
void Scene::addDrawable(argsT... params)
{
	m_objects.push_back(std::shared_ptr<Drawable>(new DrawT(params...)));
}

template <typename DrawT, typename... argsT>
void Scene::addPhysicallyDrawable(argsT... params)
{
	m_physicallObjects.push_back(std::shared_ptr<PhysicallyDrawable>(new DrawT(params...)));
}