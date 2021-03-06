// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PERIDOT_BIN_LEDGER_TESTING_TEST_WITH_COROUTINES_H_
#define PERIDOT_BIN_LEDGER_TESTING_TEST_WITH_COROUTINES_H_

#include "lib/fxl/functional/closure.h"
#include "lib/fxl/macros.h"
#include "peridot/bin/ledger/coroutine/coroutine_impl.h"
#include "peridot/lib/gtest/test_with_message_loop.h"

namespace test {

class TestWithCoroutines : public gtest::TestWithMessageLoop {
 public:
  TestWithCoroutines();

 protected:
  // Runs the given the given test code in a coroutine. Returns |true| if the
  // test has successfully termitated.
  bool RunInCoroutine(
      std::function<void(coroutine::CoroutineHandler*)> run_test);

  coroutine::CoroutineServiceImpl coroutine_service_;

 private:
  FXL_DISALLOW_COPY_AND_ASSIGN(TestWithCoroutines);
};

}  // namespace test

#endif  // PERIDOT_BIN_LEDGER_TESTING_TEST_WITH_COROUTINES_H_
