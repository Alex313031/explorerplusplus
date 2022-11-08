// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "XmlStorageHelper.h"
#include "../Helper/XMLSettings.h"
#include <objbase.h>

XmlStorageTest::XmlStorageTest()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
}

XmlStorageTest::~XmlStorageTest()
{
	CoUninitialize();
}

wil::com_ptr_nothrow<IXMLDOMDocument> LoadXmlDocument(const std::wstring &filePath)
{
	wil::com_ptr_nothrow<IXMLDOMDocument> xmlDocument;
	xmlDocument.attach(NXMLSettings::DomFromCOM());

	if (!xmlDocument)
	{
		return nullptr;
	}

	VARIANT_BOOL status;
	VARIANT variantFilePath = NXMLSettings::VariantString(filePath.c_str());
	xmlDocument->load(variantFilePath, &status);

	if (status != VARIANT_TRUE)
	{
		return nullptr;
	}

	return xmlDocument;
}

std::optional<XmlDocumentData> CreateXmlDocument()
{
	wil::com_ptr_nothrow<IXMLDOMDocument> xmlDocument;
	xmlDocument.attach(NXMLSettings::DomFromCOM());

	if (!xmlDocument)
	{
		return {};
	}

	auto tag = wil::make_bstr_nothrow(L"xml");
	auto attribute = wil::make_bstr_nothrow(L"version='1.0'");
	wil::com_ptr_nothrow<IXMLDOMProcessingInstruction> processingInstruction;
	xmlDocument->createProcessingInstruction(tag.get(), attribute.get(), &processingInstruction);
	NXMLSettings::AppendChildToParent(processingInstruction.get(), xmlDocument.get());

	auto rootTag = wil::make_bstr_nothrow(L"ExplorerPlusPlus");
	wil::com_ptr_nothrow<IXMLDOMElement> root;
	xmlDocument->createElement(rootTag.get(), &root);
	NXMLSettings::AppendChildToParent(root.get(), xmlDocument.get());

	return XmlDocumentData{ std::move(xmlDocument), std::move(root) };
}
