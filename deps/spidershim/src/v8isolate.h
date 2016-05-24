// Copyright Mozilla Foundation. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#pragma once

#include <stack>

#include "v8.h"
#include "v8context.h"
#include "rootstore.h"
#include "mozilla/Maybe.h"

namespace v8 {

namespace internal {
// Node only has 4 slots
static const uint32_t kNumIsolateDataSlots = 4;

bool InitializeIsolate();
}

struct Isolate::Impl {
  Impl()
      : rt(nullptr),
        topTryCatch(nullptr),
        serviceInterrupt(false),
        terminatingExecution(false),
        amountOfExternallyAllocatedMemory(0) {
    memset(embeddedData, 0, sizeof(embeddedData));
  }

  JSRuntime* rt;
  TryCatch* topTryCatch;
  std::vector<Context*> contexts;
  std::stack<Context*> currentContexts;
  std::vector<StackFrame*> stackFrames;
  std::vector<StackTrace*> stackTraces;
  std::vector<GCCallback> gcProlougeCallbacks;
  std::vector<GCCallback> gcEpilogueCallbacks;
  mozilla::Maybe<internal::RootStore> persistents;
  mozilla::Maybe<internal::RootStore> eternals;
  std::vector<MessageCallback> messageListeners;
  void* embeddedData[internal::kNumIsolateDataSlots];
  bool serviceInterrupt;
  bool terminatingExecution;
  int64_t amountOfExternallyAllocatedMemory;

  internal::RootStore& EnsurePersistents(Isolate* iso) {
    if (!persistents) {
      persistents.emplace(iso);
    }
    return *persistents;
  }

  internal::RootStore& EnsureEternals(Isolate* iso) {
    if (!eternals) {
      eternals.emplace(iso);
    }
    return *eternals;
  }

  static void ErrorReporter(JSContext* cx, const char* message,
                            JSErrorReport* report) {
    fprintf(stderr, "JS error at %s:%u: %s\n",
            report->filename ? report->filename : "<no filename>",
            (unsigned int)report->lineno, message);
  }

  static bool OnInterrupt(JSContext* cx);
  static void OnGC(JSRuntime *rt, JSGCStatus status, void *data);
  static bool EnqueuePromiseJobCallback(JSContext* cx, JS::HandleObject job, void* data);

};

JSContext* JSContextFromIsolate(v8::Isolate* isolate);
}
