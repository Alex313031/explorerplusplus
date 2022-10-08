// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "DataObjectImpl.h"
#include "EnumFormatEtcImpl.h"
#include <list>

DataObjectImpl::DataObjectImpl(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, int count)
{
	for (int i = 0; i < count; i++)
	{
		DataObjectInternal dao = { pFormatEtc[i], pMedium[i] };
		m_daoList.push_back(dao);
	}

	SetAsyncMode(FALSE);

	m_bInOperation = FALSE;
}

DataObjectImpl::~DataObjectImpl()
{
	for (auto dao : m_daoList)
	{
		ReleaseStgMedium(&dao.stg);
	}
}

// IDataObject
IFACEMETHODIMP DataObjectImpl::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	if (pFormatEtc == nullptr || pMedium == nullptr)
	{
		return E_INVALIDARG;
	}

	if (QueryGetData(pFormatEtc) == DV_E_FORMATETC)
	{
		return DV_E_FORMATETC;
	}

	for (const auto &dao : m_daoList)
	{
		if (dao.fe.cfFormat == pFormatEtc->cfFormat && dao.fe.tymed & pFormatEtc->tymed
			&& dao.fe.dwAspect == pFormatEtc->dwAspect)
		{
			BOOL bRet = DuplicateStorageMedium(pMedium, &dao.stg, &dao.fe);

			if (!bRet)
			{
				return STG_E_MEDIUMFULL;
			}

			return S_OK;
		}
	}

	return DV_E_FORMATETC;
}

BOOL DataObjectImpl::DuplicateStorageMedium(STGMEDIUM *pstgDest, const STGMEDIUM *pstgSrc,
	const FORMATETC *pftc)
{
	pstgDest->tymed = pstgSrc->tymed;
	pstgDest->pUnkForRelease = nullptr;

	if (pstgSrc->pUnkForRelease != nullptr)
	{
		pstgDest->pUnkForRelease = pstgSrc->pUnkForRelease;
		pstgSrc->pUnkForRelease->AddRef();
	}

	BOOL success = TRUE;

	switch (pftc->tymed)
	{
	case TYMED_HGLOBAL:
	case TYMED_FILE:
	case TYMED_GDI:
	case TYMED_MFPICT:
	case TYMED_ENHMF:
		success = DuplicateData(pstgDest, pstgSrc, pftc);
		break;

	case TYMED_ISTREAM:
		pstgDest->pstm = pstgSrc->pstm;
		pstgSrc->pstm->AddRef();
		break;

	case TYMED_ISTORAGE:
		pstgDest->pstg = pstgSrc->pstg;
		pstgSrc->pstg->AddRef();
		break;

	case TYMED_NULL:
		/* Do nothing. */
		break;
	}

	return success;
}

BOOL DataObjectImpl::DuplicateData(STGMEDIUM *pstgDest, const STGMEDIUM *pstgSrc,
	const FORMATETC *pftc)
{
	HANDLE hData = OleDuplicateData(pstgSrc->hGlobal, pftc->cfFormat, 0);

	if (hData == nullptr)
	{
		return FALSE;
	}

	switch (pftc->tymed)
	{
	case TYMED_HGLOBAL:
		pstgDest->hGlobal = hData;
		break;

	case TYMED_FILE:
		pstgDest->lpszFileName = reinterpret_cast<LPOLESTR>(hData);
		break;

	case TYMED_GDI:
		pstgDest->hBitmap = reinterpret_cast<HBITMAP>(hData);
		break;

	case TYMED_MFPICT:
		pstgDest->hMetaFilePict = hData;
		break;

	case TYMED_ENHMF:
		pstgDest->hEnhMetaFile = reinterpret_cast<HENHMETAFILE>(hData);
		break;
	}

	return TRUE;
}

IFACEMETHODIMP DataObjectImpl::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	UNREFERENCED_PARAMETER(pFormatEtc);
	UNREFERENCED_PARAMETER(pMedium);

	return DV_E_TYMED;
}

IFACEMETHODIMP DataObjectImpl::QueryGetData(FORMATETC *pFormatEtc)
{
	if (pFormatEtc == nullptr)
	{
		return E_INVALIDARG;
	}

	for (const auto &dao : m_daoList)
	{
		if (dao.fe.cfFormat == pFormatEtc->cfFormat && dao.fe.tymed & pFormatEtc->tymed
			&& dao.fe.dwAspect == pFormatEtc->dwAspect)
		{
			return S_OK;
		}
	}

	return DV_E_FORMATETC;
}

IFACEMETHODIMP DataObjectImpl::GetCanonicalFormatEtc(FORMATETC *pFormatEtcIn,
	FORMATETC *pFormatEtcOut)
{
	UNREFERENCED_PARAMETER(pFormatEtcIn);

	if (pFormatEtcOut == nullptr)
	{
		return E_INVALIDARG;
	}

	pFormatEtcOut->ptd = nullptr;

	return E_NOTIMPL;
}

IFACEMETHODIMP DataObjectImpl::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease)
{
	if (pFormatEtc == nullptr || pMedium == nullptr)
	{
		return E_INVALIDARG;
	}

	DataObjectInternal dao;

	dao.fe = *pFormatEtc;

	if (fRelease)
	{
		dao.stg = *pMedium;
	}
	else
	{
		BOOL bRet = DuplicateStorageMedium(&dao.stg, pMedium, pFormatEtc);

		if (!bRet)
		{
			return E_OUTOFMEMORY;
		}
	}

	m_daoList.push_back(dao);

	return S_OK;
}

IFACEMETHODIMP DataObjectImpl::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
	if (ppEnumFormatEtc == nullptr)
	{
		return E_INVALIDARG;
	}

	if (dwDirection == DATADIR_GET)
	{
		std::list<FORMATETC> feList;

		for (const auto &dao : m_daoList)
		{
			feList.push_back(dao.fe);
		}

		auto enumFormatEtcImpl = winrt::make_self<EnumFormatEtcImpl>(feList);
		*ppEnumFormatEtc = enumFormatEtcImpl.detach();

		return S_OK;
	}

	return E_NOTIMPL;
}

IFACEMETHODIMP DataObjectImpl::DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink,
	DWORD *pdwConnection)
{
	UNREFERENCED_PARAMETER(pFormatEtc);
	UNREFERENCED_PARAMETER(advf);
	UNREFERENCED_PARAMETER(pAdvSink);
	UNREFERENCED_PARAMETER(pdwConnection);

	return E_NOTIMPL;
}

IFACEMETHODIMP DataObjectImpl::DUnadvise(DWORD dwConnection)
{
	UNREFERENCED_PARAMETER(dwConnection);

	return OLE_E_ADVISENOTSUPPORTED;
}

IFACEMETHODIMP DataObjectImpl::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
	UNREFERENCED_PARAMETER(ppenumAdvise);

	return OLE_E_ADVISENOTSUPPORTED;
}

// IDataObjectAsyncCapability
// End operation does not seem to be called when dropping the CF_HDROP format into Windows Explorer.
// See: http://us.generation-nt.com/iasyncoperation-idataobject-help-45020022.html
IFACEMETHODIMP DataObjectImpl::EndOperation(HRESULT hResult, IBindCtx *pbcReserved, DWORD dwEffects)
{
	UNREFERENCED_PARAMETER(hResult);
	UNREFERENCED_PARAMETER(pbcReserved);
	UNREFERENCED_PARAMETER(dwEffects);

	m_bInOperation = FALSE;
	return S_OK;
}

IFACEMETHODIMP DataObjectImpl::GetAsyncMode(BOOL *pfIsOpAsync)
{
	*pfIsOpAsync = m_bDoOpAsync;

	return S_OK;
}

IFACEMETHODIMP DataObjectImpl::InOperation(BOOL *pfInAsyncOp)
{
	*pfInAsyncOp = m_bInOperation;

	return S_OK;
}

IFACEMETHODIMP DataObjectImpl::SetAsyncMode(BOOL fDoOpAsync)
{
	m_bDoOpAsync = fDoOpAsync;

	return S_OK;
}

IFACEMETHODIMP DataObjectImpl::StartOperation(IBindCtx *pbcReserved)
{
	UNREFERENCED_PARAMETER(pbcReserved);

	m_bInOperation = TRUE;

	return S_OK;
}
