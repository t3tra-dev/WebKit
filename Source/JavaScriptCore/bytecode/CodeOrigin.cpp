/*
 * Copyright (C) 2012-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "CodeOrigin.h"

#include "CodeBlock.h"
#include "InlineCallFrame.h"
#include <wtf/TZoneMallocInlines.h>

namespace JSC {

WTF_MAKE_TZONE_ALLOCATED_IMPL(CodeOrigin::OutOfLineCodeOrigin);

unsigned CodeOrigin::inlineDepth() const
{
    unsigned result = 1;
    for (InlineCallFrame* current = inlineCallFrame(); current; current = current->directCaller.inlineCallFrame())
        result++;
    return result;
}

bool CodeOrigin::isApproximatelyEqualTo(const CodeOrigin& other, InlineCallFrame* terminal) const
{
    CodeOrigin a = *this;
    CodeOrigin b = other;

    if (!a.isSet())
        return !b.isSet();
    if (!b.isSet())
        return false;
    
    if (a.isHashTableDeletedValue())
        return b.isHashTableDeletedValue();
    if (b.isHashTableDeletedValue())
        return false;
    
    for (;;) {
        ASSERT(a.isSet());
        ASSERT(b.isSet());
        
        if (a.bytecodeIndex() != b.bytecodeIndex())
            return false;

        auto* aInlineCallFrame = a.inlineCallFrame();
        auto* bInlineCallFrame = b.inlineCallFrame();
        bool aHasInlineCallFrame = !!aInlineCallFrame && aInlineCallFrame != terminal;
        bool bHasInlineCallFrame = !!bInlineCallFrame;
        if (aHasInlineCallFrame != bHasInlineCallFrame)
            return false;
        
        if (!aHasInlineCallFrame)
            return true;
        
        if (aInlineCallFrame->baselineCodeBlock.get() != bInlineCallFrame->baselineCodeBlock.get())
            return false;
        
        a = aInlineCallFrame->directCaller;
        b = bInlineCallFrame->directCaller;
    }
}

unsigned CodeOrigin::approximateHash(InlineCallFrame* terminal) const
{
    if (!isSet())
        return 0;
    if (isHashTableDeletedValue())
        return 1;
    
    unsigned result = 2;
    CodeOrigin codeOrigin = *this;
    for (;;) {
        result += codeOrigin.bytecodeIndex().asBits();

        auto* inlineCallFrame = codeOrigin.inlineCallFrame();

        if (!inlineCallFrame)
            return result;
        
        if (inlineCallFrame == terminal)
            return result;
        
        result += WTF::PtrHash<JSCell*>::hash(inlineCallFrame->baselineCodeBlock.get());
        
        codeOrigin = inlineCallFrame->directCaller;
    }
}

Vector<CodeOrigin> CodeOrigin::inlineStack() const
{
    Vector<CodeOrigin> result(inlineDepth());
    result.last() = *this;
    unsigned index = result.size() - 2;
    for (InlineCallFrame* current = inlineCallFrame(); current; current = current->directCaller.inlineCallFrame())
        result[index--] = current->directCaller;
    RELEASE_ASSERT(!result[0].inlineCallFrame());
    return result;
}

CodeBlock* CodeOrigin::codeOriginOwner() const
{
    auto* inlineCallFrame = this->inlineCallFrame();
    if (!inlineCallFrame)
        return nullptr;
    return inlineCallFrame->baselineCodeBlock.get();
}

int CodeOrigin::stackOffset() const
{
    auto* inlineCallFrame = this->inlineCallFrame();
    if (!inlineCallFrame)
        return 0;
    return inlineCallFrame->stackOffset;
}

void CodeOrigin::dump(PrintStream& out) const
{
    if (!isSet()) {
        out.print("<none>");
        return;
    }
    
    Vector<CodeOrigin> stack = inlineStack();
    for (unsigned i = 0; i < stack.size(); ++i) {
        if (i)
            out.print(" --> ");
        
        if (InlineCallFrame* frame = stack[i].inlineCallFrame()) {
            out.print(frame->briefFunctionInformation(), ":<", RawPointer(frame->baselineCodeBlock.get()), "> ");
            if (frame->isClosureCall)
                out.print("(closure) ");
        }
        
        out.print(stack[i].bytecodeIndex());
    }
}

void CodeOrigin::dumpInContext(PrintStream& out, DumpContext*) const
{
    dump(out);
}

} // namespace JSC
