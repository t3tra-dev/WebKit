// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2024 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "config.h"
#include "CSSParserTokenRange.h"

#include "StyleSheetContents.h"
#include <wtf/NeverDestroyed.h>
#include <wtf/text/StringBuilder.h>

namespace WebCore {

CSSParserToken& CSSParserTokenRange::eofToken()
{
    static NeverDestroyed<CSSParserToken> eofToken(EOFToken);
    return eofToken.get();
}

CSSParserTokenRange CSSParserTokenRange::makeSubRange(const CSSParserToken* first, const CSSParserToken* last) const
{
    if (first == &eofToken())
        first = std::to_address(m_tokens.end());

    if (last == &eofToken())
        last = std::to_address(m_tokens.end());

WTF_ALLOW_UNSAFE_BUFFER_USAGE_BEGIN
    ASSERT(first <= last);
    return { std::span { first, last } };
WTF_ALLOW_UNSAFE_BUFFER_USAGE_END
}

CSSParserTokenRange CSSParserTokenRange::makeSubRange(std::span<const CSSParserToken> subrange) const
{
    ASSERT(std::to_address(subrange.end()) <= std::to_address(m_tokens.end()));
    return { subrange };
}

CSSParserTokenRange CSSParserTokenRange::consumeBlock()
{
    ASSERT(peek().getBlockType() == CSSParserToken::BlockStart);
    auto start = m_tokens.subspan(1);
    unsigned nestingLevel = 0;
    do {
        const CSSParserToken& token = consume();
        if (token.getBlockType() == CSSParserToken::BlockStart)
            nestingLevel++;
        else if (token.getBlockType() == CSSParserToken::BlockEnd)
            nestingLevel--;
    } while (nestingLevel && !m_tokens.empty());

    if (nestingLevel)
        return makeSubRange(start.first(m_tokens.data() - start.data())); // Ended at EOF
    return makeSubRange(start.first(m_tokens.data() - start.data() - 1));
}

CSSParserTokenRange CSSParserTokenRange::consumeBlockCheckingForEditability(StyleSheetContents* styleSheet)
{
    ASSERT(peek().getBlockType() == CSSParserToken::BlockStart);
    auto start = m_tokens.subspan(1);
    unsigned nestingLevel = 0;
    do {
        const auto& token = consume();
        if (token.getBlockType() == CSSParserToken::BlockStart)
            nestingLevel++;
        else if (token.getBlockType() == CSSParserToken::BlockEnd)
            nestingLevel--;

        if (styleSheet && !styleSheet->usesStyleBasedEditability() && token.type() == IdentToken && equalLettersIgnoringASCIICase(token.value(), "-webkit-user-modify"_s))
            styleSheet->parserSetUsesStyleBasedEditability();
    } while (nestingLevel && !m_tokens.empty());

    if (nestingLevel)
        return makeSubRange(start.first(m_tokens.data() - start.data())); // Ended at EOF
    return makeSubRange(start.first(m_tokens.data() - start.data() - 1));
}

void CSSParserTokenRange::consumeComponentValue()
{
    // FIXME: This is going to do multiple passes over large sections of a stylesheet.
    // We should consider optimising this by precomputing where each block ends.
    unsigned nestingLevel = 0;
    do {
        const CSSParserToken& token = consume();
        if (token.getBlockType() == CSSParserToken::BlockStart)
            nestingLevel++;
        else if (token.getBlockType() == CSSParserToken::BlockEnd)
            nestingLevel--;
    } while (nestingLevel && !m_tokens.empty());
}

void CSSParserTokenRange::trimTrailingWhitespace()
{
    size_t i = m_tokens.size();
    for (; i > 0 && CSSTokenizer::isWhitespace(m_tokens[i - 1].type()); --i) { }
    dropLast(m_tokens, m_tokens.size() - i);
}

const CSSParserToken& CSSParserTokenRange::consumeLast()
{
    if (atEnd())
        eofToken();
    return WTF::consumeLast(m_tokens);
}

String CSSParserTokenRange::serialize(CSSParserToken::SerializationMode mode) const
{
    StringBuilder builder;
    for (size_t i = 0; i < m_tokens.size(); ++i)
        m_tokens[i].serialize(builder, (i + 1) == m_tokens.size() ? nullptr : &m_tokens[i + 1], mode);
    return builder.toString();
}

} // namespace WebCore
