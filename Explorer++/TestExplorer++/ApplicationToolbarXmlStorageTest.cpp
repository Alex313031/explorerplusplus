// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "ApplicationToolbarXmlStorage.h"
#include "Application.h"
#include "ApplicationModel.h"
#include "ApplicationToolbarStorageHelper.h"
#include "MovableModelHelper.h"
#include "ResourceHelper.h"
#include "XmlStorageHelper.h"
#include <gtest/gtest.h>

using namespace Applications;
using namespace testing;

class ApplicationToolbarXmlStorageTest : public XmlStorageTest
{
};

TEST_F(ApplicationToolbarXmlStorageTest, Load)
{
	ApplicationModel referenceModel;
	BuildLoadSaveReferenceModel(&referenceModel);

	std::wstring xmlFilePath = GetResourcePath(L"application-toolbar-config.xml");
	auto xmlDocument = LoadXmlDocument(xmlFilePath);
	ASSERT_TRUE(xmlDocument);

	ApplicationModel loadedModel;
	ApplicationToolbarXmlStorage::Load(xmlDocument.get(), &loadedModel);

	EXPECT_EQ(loadedModel, referenceModel);
}

TEST_F(ApplicationToolbarXmlStorageTest, Save)
{
	ApplicationModel referenceModel;
	BuildLoadSaveReferenceModel(&referenceModel);

	auto xmlDocumentData = CreateXmlDocument();
	ASSERT_TRUE(xmlDocumentData);

	ApplicationToolbarXmlStorage::Save(xmlDocumentData->xmlDocument.get(),
		xmlDocumentData->root.get(), &referenceModel);

	ApplicationModel loadedModel;
	ApplicationToolbarXmlStorage::Load(xmlDocumentData->xmlDocument.get(), &loadedModel);

	EXPECT_EQ(loadedModel, referenceModel);
}
