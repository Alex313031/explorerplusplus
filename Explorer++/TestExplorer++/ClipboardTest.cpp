// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "../Helper/Clipboard.h"
#include <gtest/gtest.h>

using namespace testing;

class ClipboardTest : public Test
{
protected:
	void SetUp() override
	{
		auto res = m_clipboard.Clear();
		ASSERT_TRUE(res);
	}

	Clipboard m_clipboard;
};

TEST_F(ClipboardTest, ReadWriteText)
{
	std::wstring text = L"Clipboard text";
	auto res = m_clipboard.WriteText(text);
	ASSERT_TRUE(res);

	auto clipboardText = m_clipboard.ReadText();
	ASSERT_NE(clipboardText, std::nullopt);
	EXPECT_EQ(*clipboardText, text);
}

TEST_F(ClipboardTest, ReadWriteCustomData)
{
	using namespace std::string_literals;

	// WriteCustomData() can be used to write arbitrary data, contained within a string container.
	// Here, it's being used to write text, so it's important that the NULL character is explicitly
	// included in the string (since it should be part of the data that's written).
	std::string text = "ANSI Clipboard text\0"s;
	auto res = m_clipboard.WriteCustomData(CF_TEXT, text);
	ASSERT_TRUE(res);

	auto clipboardText = m_clipboard.ReadCustomData(CF_TEXT);
	ASSERT_NE(clipboardText, std::nullopt);
	EXPECT_EQ(*clipboardText, text);
}

TEST_F(ClipboardTest, Clear)
{
	std::wstring text = L"Clipboard text";
	auto res = m_clipboard.WriteText(text);
	ASSERT_TRUE(res);

	res = m_clipboard.Clear();
	ASSERT_TRUE(res);

	// The clipboard was cleared, so an attempt to read data from it should fail.
	auto clipboardText = m_clipboard.ReadText();
	EXPECT_EQ(clipboardText, std::nullopt);
}
