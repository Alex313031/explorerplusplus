// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "pch.h"
#include "ShellEnumeratorImpl.h"
#include "ResourceTestHelper.h"
#include "../Helper/ShellHelper.h"
#include <gtest/gtest.h>
#include <wil/com.h>

using namespace testing;

class ShellEnumeratorImplTest : public Test
{
protected:
	ShellEnumeratorImplTest(ShellEnumeratorImpl::HiddenItemsPolicy hiddenItemsPolicy =
								ShellEnumeratorImpl::HiddenItemsPolicy::ExcludeHidden) :
		m_shellEnumerator(nullptr, hiddenItemsPolicy)
	{
	}

	void SetUp() override
	{
		SetTestFileHidden(true);
	}

	void TearDown() override
	{
		// Removing the hidden attribute isn't necessary for any of the tests, but if the test file
		// has the hidden attribute set when building, the xcopy post-build event will fail to
		// overwrite it (specifically, xcopy will fail with an "Access denied" error). That's the
		// reason the attribute is removed here.
		SetTestFileHidden(false);
	}

	void CheckEnumeration(const std::vector<std::wstring> &expectedItems)
	{
		std::vector<std::wstring> itemNames;
		EnumerateTestDirectory(itemNames);

		EXPECT_THAT(itemNames, UnorderedElementsAreArray(expectedItems));
	}

	void EnumerateTestDirectory(std::vector<std::wstring> &itemNames)
	{
		PidlAbsolute pidl;
		std::wstring testDirectory = GetEnumerationTestDirectory();
		ASSERT_HRESULT_SUCCEEDED(
			SHParseDisplayName(testDirectory.c_str(), nullptr, PidlOutParam(pidl), 0, nullptr));

		std::vector<PidlChild> items;
		ASSERT_HRESULT_SUCCEEDED(
			m_shellEnumerator.EnumerateDirectory(pidl.Raw(), items, m_stopSource.get_token()));

		wil::com_ptr_nothrow<IShellFolder> parent;
		ASSERT_HRESULT_SUCCEEDED(
			SHBindToObject(nullptr, pidl.Raw(), nullptr, IID_PPV_ARGS(&parent)));

		for (const auto &item : items)
		{
			std::wstring name;
			ASSERT_HRESULT_SUCCEEDED(
				GetDisplayName(parent.get(), item.Raw(), SHGDN_INFOLDER | SHGDN_FORPARSING, name));

			itemNames.push_back(name);
		}
	}

	std::stop_source m_stopSource;

private:
	std::filesystem::path GetEnumerationTestDirectory()
	{
		return GetResourcePath(L"EnumerationTestDirectory");
	}

	void SetTestFileHidden(bool set)
	{
		auto testDirectory = GetEnumerationTestDirectory();
		auto hiddenItemPath = testDirectory / L"hidden.txt";
		auto attributes = GetFileAttributes(hiddenItemPath.c_str());
		ASSERT_NE(attributes, INVALID_FILE_ATTRIBUTES);

		if (set)
		{
			WI_SetFlag(attributes, FILE_ATTRIBUTE_HIDDEN);
		}
		else
		{
			WI_ClearFlag(attributes, FILE_ATTRIBUTE_HIDDEN);
		}

		auto res = SetFileAttributes(hiddenItemPath.c_str(), attributes);
		ASSERT_NE(res, 0);
	}

	ShellEnumeratorImpl m_shellEnumerator;
};

TEST_F(ShellEnumeratorImplTest, EnumerateTestDirectory)
{
	CheckEnumeration({ L"item1.txt", L"item2.txt", L"item3.txt" });
}

TEST_F(ShellEnumeratorImplTest, StopToken)
{
	m_stopSource.request_stop();

	// A stop was requested, so it's expected that the enumeration will stop early and that no items
	// will be returned.
	std::vector<std::wstring> itemNames;
	EnumerateTestDirectory(itemNames);
	EXPECT_TRUE(itemNames.empty());
}

class ShellEnumeratorImplIncludeHiddenTest : public ShellEnumeratorImplTest
{
protected:
	ShellEnumeratorImplIncludeHiddenTest() :
		ShellEnumeratorImplTest(ShellEnumeratorImpl::HiddenItemsPolicy::IncludeHidden)
	{
	}
};

TEST_F(ShellEnumeratorImplIncludeHiddenTest, EnumerateTestDirectory)
{
	CheckEnumeration({ L"item1.txt", L"item2.txt", L"item3.txt", L"hidden.txt" });
}
