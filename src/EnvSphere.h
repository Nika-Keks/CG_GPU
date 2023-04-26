#pragma once
#include <array>
#include "Drawable.h"
#include "Camera.h"

class EnvSphere :
	public Drawable
{
private:
	static constexpr size_t s_vSamplingSize = 30;
	static constexpr size_t s_hSamplingSize = 30;
	Camera const* m_pCamera;

public:
	EnvSphere(DirectX::XMVECTOR const& position, float radius, Camera const& camera, com_ptr<ID3D11ShaderResourceView>const& pEnvCubeMapSRV);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) override;
	void resetEndCubeMapSRV(com_ptr<ID3D11ShaderResourceView>const& pEnvCubeMapSRV);

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
	};

	void initResource(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	void updateModelBuffer(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;

	std::array<size_t, (s_vSamplingSize - 1)* (s_hSamplingSize - 1) * 6> m_vIndices;
	std::array<Vertex, s_vSamplingSize* s_hSamplingSize> m_vertices;
	DirectX::XMMATRIX m_transform;

	//environment texture
	Microsoft::WRL::ComPtr<ID3D11Resource> m_Texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	com_ptr<ID3D11PixelShader> m_pPixelShader;
	com_ptr<ID3D11ShaderResourceView> m_pEnvCubeMapSRV;
};