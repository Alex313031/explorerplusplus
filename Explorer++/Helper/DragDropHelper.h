// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "DataExchangeHelper.h"
#include <wil/result.h>
#include <ShlObj.h>
#include <shtypes.h>

STGMEDIUM GetStgMediumForGlobal(HGLOBAL global);
STGMEDIUM GetStgMediumForStream(IStream *stream);
HRESULT SetPreferredDropEffect(IDataObject *dataObject, DWORD effect);
HRESULT CreateDataObjectForShellTransfer(const std::vector<PCIDLIST_ABSOLUTE> &items,
	IDataObject **dataObjectOut);
HRESULT SetDropDescription(IDataObject *dataObject, DROPIMAGETYPE type, const std::wstring &message,
	const std::wstring &insert);

template <typename T>
HRESULT SetBlobData(IDataObject *dataObject, CLIPFORMAT format, const T &data)
{
	FORMATETC ftc = { format, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	auto global = WriteDataToGlobal(&data, sizeof(data));

	if (!global)
	{
		return E_FAIL;
	}

	STGMEDIUM stg = GetStgMediumForGlobal(global.get());
	RETURN_IF_FAILED(dataObject->SetData(&ftc, &stg, TRUE));

	// The IDataObject instance has taken ownership of stg at this point, so it's responsible for
	// freeing the data.
	global.release();

	return S_OK;
}
