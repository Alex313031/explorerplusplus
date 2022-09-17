// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "NewMenuClient.h"
#include "CoreInterface.h"
#include "ShellBrowser/ShellBrowser.h"
#include "../Helper/Logging.h"

NewMenuClient::NewMenuClient(CoreInterface *coreInterface) : m_coreInterface(coreInterface)
{
}

// INewMenuClient
IFACEMETHODIMP NewMenuClient::IncludeItems(NMCII_FLAGS *flags)
{
	/* flags will be one of:
	NMCII_ITEMS
	NMCII_FOLDERS
	Which one of these is selected determines which items are shown on the 'new' menu.
	If NMCII_ITEMS is selected, only files will be shown (meaning 'New Folder' will NOT appear).
	If NMCII_FOLDERS is selected, only folders will be shown (this means that in most cases, only
	'New Folder' and 'New Briefcase' will be shown).
	Therefore, to get all the items, OR the two flags together to show files and folders. */

	*flags = NMCII_ITEMS | NMCII_FOLDERS;

	return S_OK;
}

IFACEMETHODIMP NewMenuClient::SelectAndEditItem(PCIDLIST_ABSOLUTE pidlItem, NMCSAEI_FLAGS flags)
{
	switch (flags)
	{
	// This would usually cause the item to be selected first, then renamed. In this case however,
	// the item is selected and renamed in one operation, so this state can be ignored.
	case NMCSAEI_SELECT:
		break;

	// Now, start an in-place rename of the item.
	case NMCSAEI_EDIT:
	case NMCSAEI_EDIT_WINDOWS_10:
		LOG(info) << _T("Starting in-place rename of item created via new menu");
		m_coreInterface->GetActiveShellBrowser()->QueueRename(pidlItem);
		break;
	}

	return S_OK;
}
