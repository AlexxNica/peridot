// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PERIDOT_BIN_CLOUD_PROVIDER_FIREBASE_FIREBASE_EVENT_STREAM_H_
#define PERIDOT_BIN_CLOUD_PROVIDER_FIREBASE_FIREBASE_EVENT_STREAM_H_

#include <functional>
#include <memory>
#include <string>

#include "lib/fsl/socket/socket_drainer.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/strings/string_view.h"
#include "peridot/bin/cloud_provider_firebase/firebase/status.h"
#include "peridot/bin/ledger/callback/destruction_sentinel.h"

namespace firebase {

// TODO(ppi): Use a client interface instead.
using EventCallback = void(Status status,
                           const std::string& event,
                           const std::string& data);
using CompletionCallback = void();

// Socket drainer that parses a stream of Server-Sent Events.
// Data format of the stream is specified in http://www.w3.org/TR/eventsource/.
class EventStream : public fsl::SocketDrainer::Client {
 public:
  EventStream();
  ~EventStream() override;

  void Start(zx::socket source,
             const std::function<EventCallback>& event_callback,
             const std::function<CompletionCallback>& completion_callback);

 private:
  friend class EventStreamTest;

  // fsl::SocketDrainer::Client:
  void OnDataAvailable(const void* data, size_t num_bytes) override;
  void OnDataComplete() override;

  // Returns false if the object has been destroyed within this method.
  bool ProcessLine(fxl::StringView line);

  void ProcessField(fxl::StringView field, fxl::StringView value);

  std::function<EventCallback> event_callback_;
  std::function<CompletionCallback> completion_callback_;

  // Unprocessed part of the current line.
  std::string pending_line_;
  std::string data_;
  std::string event_type_;

  std::unique_ptr<fsl::SocketDrainer> drainer_;

  callback::DestructionSentinel destruction_sentinel_;

  FXL_DISALLOW_COPY_AND_ASSIGN(EventStream);
};

}  // namespace firebase

#endif  // PERIDOT_BIN_CLOUD_PROVIDER_FIREBASE_FIREBASE_EVENT_STREAM_H_