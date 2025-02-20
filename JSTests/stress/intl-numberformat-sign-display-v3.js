// Copyright 2021 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
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

function shouldBe(actual, expected) {
    if (actual !== expected)
        throw new Error('bad value: ' + actual + " " + expected);
}

// Test default.
let nf = new Intl.NumberFormat();
shouldBe("auto", nf.resolvedOptions().signDisplay);

nf = new Intl.NumberFormat("en");
shouldBe("auto", nf.resolvedOptions().signDisplay);

const testData = [
    ["auto",        "-123",  "-0",  "0",  "123"],
    ["always",      "-123",  "-0", "+0", "+123"],
    ["never",       "123",   "0",  "0",  "123"],
    ["exceptZero",  "-123",  "0",  "0",  "+123"],
    ["negative",    "-123",  "0",  "0",  "123"],
];

for (const [signDisplay, neg, negZero, zero, pos] of testData) {
    nf = new Intl.NumberFormat("en", {signDisplay});
    shouldBe(signDisplay, nf.resolvedOptions().signDisplay);
    shouldBe(neg, nf.format(-123));
    shouldBe(negZero, nf.format(-0));
    shouldBe(zero, nf.format(0));
    shouldBe(pos, nf.format(123));
}
