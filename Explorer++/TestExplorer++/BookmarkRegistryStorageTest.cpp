// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "Bookmarks/BookmarkRegistryStorage.h"
#include "BookmarkStorageHelper.h"
#include "Bookmarks/BookmarkTree.h"
#include "RegistryStorageHelper.h"
#include <gtest/gtest.h>

using namespace testing;

class BookmarkRegistryStorageTest : public RegistryStorageTest
{
protected:
	void PerformLoadTest(const std::wstring &filename, BookmarkTree *referenceBookmarkTree,
		bool compareGuids)
	{
		ImportRegistryResource(filename);

		BookmarkTree loadedBookmarkTree;
		BookmarkRegistryStorage::Load(APPLICATION_TEST_KEY, &loadedBookmarkTree);

		CompareBookmarkTrees(&loadedBookmarkTree, referenceBookmarkTree, compareGuids);
	}
};

TEST_F(BookmarkRegistryStorageTest, V2Load)
{
	BookmarkTree referenceBookmarkTree;
	BuildV2LoadSaveReferenceTree(&referenceBookmarkTree);

	PerformLoadTest(L"bookmarks-v2.reg", &referenceBookmarkTree, true);
}

TEST_F(BookmarkRegistryStorageTest, V2Save)
{
	BookmarkTree referenceBookmarkTree;
	BuildV2LoadSaveReferenceTree(&referenceBookmarkTree);

	BookmarkRegistryStorage::Save(APPLICATION_TEST_KEY, &referenceBookmarkTree);

	BookmarkTree loadedBookmarkTree;
	BookmarkRegistryStorage::Load(APPLICATION_TEST_KEY, &loadedBookmarkTree);

	CompareBookmarkTrees(&loadedBookmarkTree, &referenceBookmarkTree, true);
}

TEST_F(BookmarkRegistryStorageTest, V1BasicLoad)
{
	BookmarkTree referenceBookmarkTree;
	BuildV1BasicLoadReferenceTree(&referenceBookmarkTree);

	PerformLoadTest(L"bookmarks-v1.reg", &referenceBookmarkTree, false);
}

TEST_F(BookmarkRegistryStorageTest, V1NestedShowOnToolbarLoad)
{
	BookmarkTree referenceBookmarkTree;
	BuildV1NestedShowOnToolbarLoadReferenceTree(&referenceBookmarkTree);

	PerformLoadTest(L"bookmarks-v1-nested-show-on-toolbar.reg", &referenceBookmarkTree, false);
}
