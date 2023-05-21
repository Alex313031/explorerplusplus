// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "DestroyFilesDialog.h"
#include "Explorer++_internal.h"
#include "MainResource.h"
#include "../Helper/Helper.h"
#include "../Helper/Macros.h"
#include "../Helper/RegistrySettings.h"
#include "../Helper/StringHelper.h"
#include "../Helper/XMLSettings.h"

const TCHAR DestroyFilesDialogPersistentSettings::SETTINGS_KEY[] = _T("DestroyFiles");

const TCHAR DestroyFilesDialogPersistentSettings::SETTING_OVERWRITE_METHOD[] =
	_T("OverwriteMethod");

DestroyFilesDialog::DestroyFilesDialog(HINSTANCE resourceInstance, HWND hParent,
	const std::list<std::wstring> &FullFilenameList, BOOL bShowFriendlyDates) :
	ThemedDialog(resourceInstance, IDD_DESTROYFILES, hParent, DialogSizingType::Both)
{
	m_FullFilenameList = FullFilenameList;
	m_bShowFriendlyDates = bShowFriendlyDates;

	m_pdfdps = &DestroyFilesDialogPersistentSettings::GetInstance();
}

INT_PTR DestroyFilesDialog::OnInitDialog()
{
	m_icon.reset(LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_MAIN)));
	SetClassLongPtr(m_hDlg, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(m_icon.get()));

	HWND hListView = GetDlgItem(m_hDlg, IDC_DESTROYFILES_LISTVIEW);

	HIMAGELIST himlSmall;
	Shell_GetImageLists(nullptr, &himlSmall);
	ListView_SetImageList(hListView, himlSmall, LVSIL_SMALL);

	ListView_SetExtendedListViewStyleEx(hListView,
		LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES,
		LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LVCOLUMN lvColumn;
	TCHAR szTemp[128];

	LoadString(GetResourceInstance(), IDS_DESTROY_FILES_COLUMN_FILE, szTemp, SIZEOF_ARRAY(szTemp));
	lvColumn.mask = LVCF_TEXT;
	lvColumn.pszText = szTemp;
	ListView_InsertColumn(hListView, 0, &lvColumn);

	LoadString(GetResourceInstance(), IDS_DESTROY_FILES_COLUMN_TYPE, szTemp, SIZEOF_ARRAY(szTemp));
	lvColumn.mask = LVCF_TEXT;
	lvColumn.pszText = szTemp;
	ListView_InsertColumn(hListView, 1, &lvColumn);

	LoadString(GetResourceInstance(), IDS_DESTROY_FILES_COLUMN_SIZE, szTemp, SIZEOF_ARRAY(szTemp));
	lvColumn.mask = LVCF_TEXT;
	lvColumn.pszText = szTemp;
	ListView_InsertColumn(hListView, 2, &lvColumn);

	LoadString(GetResourceInstance(), IDS_DESTROY_FILES_COLUMN_DATE_MODIFIED, szTemp,
		SIZEOF_ARRAY(szTemp));
	lvColumn.mask = LVCF_TEXT;
	lvColumn.pszText = szTemp;
	ListView_InsertColumn(hListView, 3, &lvColumn);

	int iItem = 0;

	for (const auto &strFullFilename : m_FullFilenameList)
	{
		TCHAR szFullFilename[MAX_PATH];

		StringCchCopy(szFullFilename, SIZEOF_ARRAY(szFullFilename), strFullFilename.c_str());

		/* TODO: Perform in background thread. */
		SHFILEINFO shfi;
		SHGetFileInfo(szFullFilename, 0, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_TYPENAME);

		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
		lvItem.iItem = iItem;
		lvItem.iSubItem = 0;
		lvItem.pszText = szFullFilename;
		lvItem.iImage = shfi.iIcon;
		ListView_InsertItem(hListView, &lvItem);

		ListView_SetItemText(hListView, iItem, 1, shfi.szTypeName);

		WIN32_FILE_ATTRIBUTE_DATA wfad;
		GetFileAttributesEx(szFullFilename, GetFileExInfoStandard, &wfad);

		ULARGE_INTEGER fileSize = { wfad.nFileSizeLow, wfad.nFileSizeHigh };
		auto fileSizeText = FormatSizeString(fileSize.QuadPart);
		ListView_SetItemText(hListView, iItem, 2, fileSizeText.data());

		TCHAR szDateModified[32];
		CreateFileTimeString(&wfad.ftLastWriteTime, szDateModified, SIZEOF_ARRAY(szDateModified),
			m_bShowFriendlyDates);
		ListView_SetItemText(hListView, iItem, 3, szDateModified);

		iItem++;
	}

	ListView_SetColumnWidth(hListView, 0, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hListView, 1, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hListView, 2, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(hListView, 3, LVSCW_AUTOSIZE_USEHEADER);

	switch (m_pdfdps->m_overwriteMethod)
	{
	case NFileOperations::OverwriteMethod::OnePass:
		CheckDlgButton(m_hDlg, IDC_DESTROYFILES_RADIO_ONEPASS, BST_CHECKED);
		break;

	case NFileOperations::OverwriteMethod::ThreePass:
		CheckDlgButton(m_hDlg, IDC_DESTROYFILES_RADIO_THREEPASS, BST_CHECKED);
		break;
	}

	m_pdfdps->RestoreDialogPosition(m_hDlg, true);

	return 0;
}

std::vector<ResizableDialogControl> DestroyFilesDialog::GetResizableControls()
{
	std::vector<ResizableDialogControl> controls;
	controls.emplace_back(GetDlgItem(m_hDlg, IDC_DESTROYFILES_LISTVIEW), MovingType::None,
		SizingType::Both);
	controls.emplace_back(GetDlgItem(m_hDlg, IDC_GROUP_WIPE_METHOD), MovingType::Vertical,
		SizingType::Horizontal);
	controls.emplace_back(GetDlgItem(m_hDlg, IDC_DESTROYFILES_RADIO_ONEPASS), MovingType::Vertical,
		SizingType::None);
	controls.emplace_back(GetDlgItem(m_hDlg, IDC_DESTROYFILES_RADIO_THREEPASS),
		MovingType::Vertical, SizingType::None);
	controls.emplace_back(GetDlgItem(m_hDlg, IDC_DESTROYFILES_STATIC_WARNING_MESSAGE),
		MovingType::Vertical, SizingType::Horizontal);
	controls.emplace_back(GetDlgItem(m_hDlg, IDOK), MovingType::Both, SizingType::None);
	controls.emplace_back(GetDlgItem(m_hDlg, IDCANCEL), MovingType::Both, SizingType::None);
	return controls;
}

INT_PTR DestroyFilesDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
	case IDOK:
		OnOk();
		break;

	case IDCANCEL:
		OnCancel();
		break;
	}

	return 0;
}

INT_PTR DestroyFilesDialog::OnClose()
{
	EndDialog(m_hDlg, 0);
	return 0;
}

void DestroyFilesDialog::SaveState()
{
	m_pdfdps->SaveDialogPosition(m_hDlg);

	if (IsDlgButtonChecked(m_hDlg, IDC_DESTROYFILES_RADIO_ONEPASS) == BST_CHECKED)
	{
		m_pdfdps->m_overwriteMethod = NFileOperations::OverwriteMethod::OnePass;
	}
	else
	{
		m_pdfdps->m_overwriteMethod = NFileOperations::OverwriteMethod::ThreePass;
	}

	m_pdfdps->m_bStateSaved = TRUE;
}

void DestroyFilesDialog::OnOk()
{
	TCHAR szConfirmation[128];
	LoadString(GetResourceInstance(), IDS_DESTROY_FILES_CONFIRMATION, szConfirmation,
		SIZEOF_ARRAY(szConfirmation));

	/* The default button in this message box will be the second
	button (i.e. the no button). */
	int iRes = MessageBox(m_hDlg, szConfirmation, NExplorerplusplus::APP_NAME,
		MB_ICONWARNING | MB_SETFOREGROUND | MB_YESNO | MB_DEFBUTTON2);

	switch (iRes)
	{
	case IDYES:
		OnConfirmDestroy();
		break;

	default:
		EndDialog(m_hDlg, 0);
		break;
	}
}

void DestroyFilesDialog::OnCancel()
{
	EndDialog(m_hDlg, 0);
}

void DestroyFilesDialog::OnConfirmDestroy()
{
	NFileOperations::OverwriteMethod overwriteMethod;

	if (IsDlgButtonChecked(m_hDlg, IDC_DESTROYFILES_RADIO_ONEPASS) == BST_CHECKED)
	{
		overwriteMethod = NFileOperations::OverwriteMethod::OnePass;
	}
	else
	{
		overwriteMethod = NFileOperations::OverwriteMethod::ThreePass;
	}

	/* TODO: Perform in background thread. */
	for (const auto &strFullFilename : m_FullFilenameList)
	{
		DeleteFileSecurely(strFullFilename, overwriteMethod);
	}

	EndDialog(m_hDlg, 1);
}

DestroyFilesDialogPersistentSettings::DestroyFilesDialogPersistentSettings() :
	DialogSettings(SETTINGS_KEY)
{
	m_overwriteMethod = NFileOperations::OverwriteMethod::OnePass;
}

DestroyFilesDialogPersistentSettings &DestroyFilesDialogPersistentSettings::GetInstance()
{
	static DestroyFilesDialogPersistentSettings dfdps;
	return dfdps;
}

void DestroyFilesDialogPersistentSettings::SaveExtraRegistrySettings(HKEY hKey)
{
	RegistrySettings::SaveDword(hKey, SETTING_OVERWRITE_METHOD,
		static_cast<DWORD>(m_overwriteMethod));
}

void DestroyFilesDialogPersistentSettings::LoadExtraRegistrySettings(HKEY hKey)
{
	DWORD value;
	RegistrySettings::Read32BitValueFromRegistry(hKey, SETTING_OVERWRITE_METHOD, value);
	m_overwriteMethod = static_cast<NFileOperations::OverwriteMethod>(value);
}

void DestroyFilesDialogPersistentSettings::SaveExtraXMLSettings(IXMLDOMDocument *pXMLDom,
	IXMLDOMElement *pParentNode)
{
	NXMLSettings::AddAttributeToNode(pXMLDom, pParentNode, SETTING_OVERWRITE_METHOD,
		NXMLSettings::EncodeIntValue(static_cast<int>(m_overwriteMethod)));
}

void DestroyFilesDialogPersistentSettings::LoadExtraXMLSettings(BSTR bstrName, BSTR bstrValue)
{
	if (lstrcmpi(bstrName, SETTING_OVERWRITE_METHOD) == 0)
	{
		m_overwriteMethod =
			static_cast<NFileOperations::OverwriteMethod>(NXMLSettings::DecodeIntValue(bstrValue));
	}
}
