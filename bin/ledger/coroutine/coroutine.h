// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PERIDOT_BIN_LEDGER_COROUTINE_COROUTINE_H_
#define PERIDOT_BIN_LEDGER_COROUTINE_COROUTINE_H_

#include <functional>

#include "lib/fxl/functional/auto_call.h"
#include "lib/fxl/functional/make_copyable.h"
#include "peridot/lib/callback/capture.h"

// This Coroutine library allows to use coroutines. A coroutine is a function
// that can interrupt itself by yielding, and the computation will resume at the
// same point when another context of execution continues the coroutine using
// its handler.
namespace coroutine {

// The Handler of a coroutine. It allows a coroutine to yield and another
// context of execution to resume the computation.
//
// Threading: until the first Yield(), the coroutine executes on the thread that
// called CoroutineService::StartCoroutine(). Between Yield() and Continue(),
// the handler can be passed to another thread - the computation resumes on the
// thread that called Continue().
class CoroutineHandler {
 public:
  virtual ~CoroutineHandler() {}

  // Yield the current coroutine. This must only be called from inside the
  // coroutine associated with this handler. If Yield returns |true|, the
  // coroutine must unwind its stack and terminate.
  FXL_WARN_UNUSED_RESULT virtual bool Yield() = 0;

  // Restarts the computation of the coroutine associated with this handler.
  // This must only be called outside of the coroutine when it is yielded. If
  // |interrupt| is true, |Yield| will return |true| when the coroutine is
  // resumed, asking it to terminate.
  virtual void Continue(bool interrupt) = 0;
};

// The service handling coroutines. It allows to create new coroutines.
// Destructing the service will terminate all active coroutines. All the
// non-terminated coroutines will eventually be activated and asked to
// terminate.
class CoroutineService {
 public:
  virtual ~CoroutineService() {}

  // Starts a new coroutine that will execute |runnable|.
  virtual void StartCoroutine(
      std::function<void(CoroutineHandler*)> runnable) = 0;
};

// Allows to execute an asynchronous call in a coroutine. The coroutine will
// yield until the asynchronous call terminates, it will then be continued and
// will store the results of the asynchronous calls in |parameters|. If
// |SyncCall| returns |true|, the coroutine must unwind its stack and terminate.
//
// |async_call| will be never be called after this method returns. As such, it
// can capture local variables by reference.
template <typename A, typename... Args>
FXL_WARN_UNUSED_RESULT bool SyncCall(CoroutineHandler* handler,
                                     const A& async_call,
                                     Args*... parameters) {
  class TerminationSentinel
      : public fxl::RefCountedThreadSafe<TerminationSentinel> {
   public:
    bool terminated = false;
  };

  auto termination_sentinel = fxl::AdoptRef(new TerminationSentinel());
  auto on_return = fxl::MakeAutoCall(
      [termination_sentinel] { termination_sentinel->terminated = true; });

  volatile bool sync_state = true;
  volatile bool callback_called = false;
  // Unblock the coroutine (by having it return early) if the asynchronous call
  // drops its callback without ever calling it.
  auto unblocker =
      fxl::MakeAutoCall([termination_sentinel, &handler, &sync_state] {
        if (termination_sentinel->terminated) {
          return;
        }

        if (sync_state) {
          sync_state = false;
          return;
        }
        handler->Continue(true);
      });
  async_call(callback::Capture(
      fxl::MakeCopyable([termination_sentinel, &sync_state, &callback_called,
                         handler, unblocker = std::move(unblocker)]() mutable {
        if (termination_sentinel->terminated) {
          return;
        }

        unblocker.cancel();
        callback_called = true;
        if (sync_state) {
          sync_state = false;
          return;
        }
        handler->Continue(false);
      }),
      parameters...));
  // If sync_state is still true, the callback was not called. Yield until it
  // is.
  if (sync_state) {
    sync_state = false;
    return handler->Yield();
  }
  return !callback_called;
};

}  // namespace coroutine

#endif  // PERIDOT_BIN_LEDGER_COROUTINE_COROUTINE_H_
