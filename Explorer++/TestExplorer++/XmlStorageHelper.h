// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include <gtest/gtest.h>
#include <wil/com.h>
#include <msxml.h>
#include <optional>
#include <string>

class XmlStorageTest : public testing::Test
{
protected:
	XmlStorageTest();
	~XmlStorageTest();
};

struct XmlDocumentData
{
	wil::com_ptr_nothrow<IXMLDOMDocument> xmlDocument;
	wil::com_ptr_nothrow<IXMLDOMElement> root;
};

wil::com_ptr_nothrow<IXMLDOMDocument> LoadXmlDocument(const std::wstring &filePath);
std::optional<XmlDocumentData> CreateXmlDocument();
