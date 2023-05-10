#include "GfxObject.h"

GfxObject::GfxObject(com_ptr<ID3D11Device> const& pDevice, com_ptr<ID3D11DeviceContext> const& pContext, com_ptr<ID3DUserDefinedAnnotation> const& pAnnotation)
	:
	m_pDevice(pDevice),
	m_pContext(pContext),
	m_pAnnotation(pAnnotation)
{
}

GfxObject::~GfxObject() {}

void GfxObject::startEvent(LPCWSTR eventName) const
{
#ifdef _DEBUG
	m_pAnnotation->BeginEvent(eventName);
#endif
}

void GfxObject::endEvent() const
{
#ifdef _DEBUG
	m_pAnnotation->EndEvent();
#endif
}

GfxObject::GtxObjError::GtxObjError(int line, const char* file)
	: BaseException(line, file)
{
}

const char* GfxObject::GtxObjError::GetType() const noexcept
{
	return "Graphic pbject error";
}
