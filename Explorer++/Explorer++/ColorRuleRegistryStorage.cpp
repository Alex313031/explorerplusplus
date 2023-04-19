// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "ColorRule.h"
#include "ColorRuleModel.h"
#include "../Helper/RegistrySettings.h"

namespace ColorRuleRegistryStorage
{

namespace
{

const TCHAR COLOR_RULES_KEY_PATH[] = _T("ColorRules");

const TCHAR SETTING_DESCRIPTION[] = _T("Description");
const TCHAR SETTING_FILENAME_PATTERN[] = _T("FilenamePattern");
const TCHAR SETTING_CASE_INSENSITIVE[] = _T("CaseInsensitive");
const TCHAR SETTING_ATTRIBUTES[] = _T("Attributes");
const TCHAR SETTING_COLOR[] = _T("Color");

std::unique_ptr<ColorRule> LoadColorRule(HKEY key)
{
	std::wstring description;
	LSTATUS result = RegistrySettings::ReadString(key, SETTING_DESCRIPTION, description);

	if (result != ERROR_SUCCESS)
	{
		return nullptr;
	}

	std::wstring filenamePattern;
	result = RegistrySettings::ReadString(key, SETTING_FILENAME_PATTERN, filenamePattern);

	if (result != ERROR_SUCCESS)
	{
		return nullptr;
	}

	bool caseInsensitive;
	result = RegistrySettings::Read32BitValueFromRegistry(key, SETTING_CASE_INSENSITIVE,
		caseInsensitive);

	if (result != ERROR_SUCCESS)
	{
		return nullptr;
	}

	DWORD attributes;
	result = RegistrySettings::ReadDword(key, SETTING_ATTRIBUTES, attributes);

	if (result != ERROR_SUCCESS)
	{
		return nullptr;
	}

	COLORREF color;
	result = RegistrySettings::ReadBinaryValue(key, SETTING_COLOR, &color, sizeof(color));

	if (result != ERROR_SUCCESS)
	{
		return nullptr;
	}

	return std::make_unique<ColorRule>(description, filenamePattern, caseInsensitive, attributes,
		color);
}

void LoadFromKey(HKEY parentKey, ColorRuleModel *model)
{
	wil::unique_hkey childKey;
	size_t index = 0;

	while (RegOpenKeyEx(parentKey, std::to_wstring(index).c_str(), 0, KEY_READ, &childKey)
		== ERROR_SUCCESS)
	{
		auto colorRule = LoadColorRule(childKey.get());

		if (colorRule)
		{
			model->AddItem(std::move(colorRule));
		}

		index++;
	}
}

void SaveColorRule(HKEY key, const ColorRule *colorRule)
{
	RegistrySettings::SaveString(key, SETTING_DESCRIPTION, colorRule->GetDescription());
	RegistrySettings::SaveString(key, SETTING_FILENAME_PATTERN, colorRule->GetFilterPattern());
	RegistrySettings::SaveDword(key, SETTING_CASE_INSENSITIVE,
		colorRule->GetFilterPatternCaseInsensitive());
	RegistrySettings::SaveDword(key, SETTING_ATTRIBUTES, colorRule->GetFilterAttributes());

	COLORREF color = colorRule->GetColor();
	RegistrySettings::SaveBinaryValue(key, SETTING_COLOR, reinterpret_cast<const BYTE *>(&color),
		sizeof(color));
}

void SaveToKey(HKEY parentKey, const ColorRuleModel *model)
{
	size_t index = 0;

	for (const auto &colorRule : model->GetItems())
	{
		wil::unique_hkey childKey;
		LSTATUS res = RegCreateKeyEx(parentKey, std::to_wstring(index).c_str(), 0, nullptr,
			REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &childKey, nullptr);

		if (res == ERROR_SUCCESS)
		{
			SaveColorRule(childKey.get(), colorRule.get());

			index++;
		}
	}
}

}

void Load(const std::wstring &mainKeyPath, ColorRuleModel *model)
{
	wil::unique_hkey colorRulesKey;
	std::wstring fullKeyPath = mainKeyPath + L"\\" + COLOR_RULES_KEY_PATH;
	LSTATUS res = RegOpenKeyEx(HKEY_CURRENT_USER, fullKeyPath.c_str(), 0, KEY_READ, &colorRulesKey);

	if (res == ERROR_SUCCESS)
	{
		model->RemoveAllItems();

		LoadFromKey(colorRulesKey.get(), model);
	}
}

void Save(const std::wstring &mainKeyPath, const ColorRuleModel *model)
{
	std::wstring fullKeyPath = mainKeyPath + L"\\" + COLOR_RULES_KEY_PATH;
	SHDeleteKey(HKEY_CURRENT_USER, fullKeyPath.c_str());

	wil::unique_hkey colorRulesKey;
	LSTATUS res = RegCreateKeyEx(HKEY_CURRENT_USER, fullKeyPath.c_str(), 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &colorRulesKey, nullptr);

	if (res == ERROR_SUCCESS)
	{
		SaveToKey(colorRulesKey.get(), model);
	}
}

}
