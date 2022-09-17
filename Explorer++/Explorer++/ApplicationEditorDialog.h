// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "Application.h"
#include "DarkModeDialogBase.h"
#include <memory>
#include <optional>

namespace Applications
{

class ApplicationModel;

class ApplicationEditorDialog : public DarkModeDialogBase
{
public:
	class EditDetails
	{
	private:
		struct Private
		{
			Private() = default;
		};

	public:
		enum class Type
		{
			ExistingItem,
			NewItem
		};

		EditDetails(Type type, Private) : type(type)
		{
		}

		static std::unique_ptr<EditDetails> AddNewApplication(
			std::unique_ptr<Application> application, std::optional<size_t> index = std::nullopt)
		{
			auto editDetails = std::make_unique<EditDetails>(Type::NewItem, Private());
			editDetails->newApplication = std::move(application);
			editDetails->index = index;
			return editDetails;
		}

		static std::unique_ptr<EditDetails> EditApplication(Application *application)
		{
			auto editDetails = std::make_unique<EditDetails>(Type::ExistingItem, Private());
			editDetails->existingApplication = application;
			return editDetails;
		}

		const Type type;

		std::unique_ptr<Application> newApplication;
		std::optional<size_t> index;

		Application *existingApplication = nullptr;
	};

	ApplicationEditorDialog(HWND parent, HMODULE resourceModule, ApplicationModel *model,
		std::unique_ptr<EditDetails> editDetails);

protected:
	INT_PTR OnInitDialog() override;
	INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
	INT_PTR OnClose() override;

private:
	void OnChooseFile();

	void OnOk();
	void OnCancel();

	void ApplyEdits(const std::wstring &newName, const std::wstring &newCommand,
		bool newShowNameOnToolbar);

	ApplicationModel *m_model;
	std::unique_ptr<EditDetails> m_editDetails;
};

}
