#pragma once
#include <d3d11_1.h>
#include "WinDef.h"
#include "BaseException.h"

class GfxObject
{
protected:
	
	class GtxObjError : public BaseException
	{
	public:
		GtxObjError(int line, const char* file);
		const char* GetType() const noexcept override;
	};

	com_ptr<ID3D11Device> m_pDevice;
	com_ptr<ID3D11DeviceContext> m_pContext;
	com_ptr<ID3DUserDefinedAnnotation> m_pAnnotation;

public:
	GfxObject(com_ptr<ID3D11Device> const& pDevice,
			com_ptr<ID3D11DeviceContext> const& pContext,
			com_ptr< ID3DUserDefinedAnnotation> const& pAnnotation);

	virtual ~GfxObject() = 0;

	void startEvent(LPCWSTR) const;
	void endEvent() const;
};