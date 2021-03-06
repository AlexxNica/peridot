// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "peridot/bin/cloud_provider_firebase/page_handler/public/record.h"

namespace cloud_provider_firebase {

Record::Record() = default;

Record::Record(Commit n, std::string t, int position, int size)
    : commit(std::move(n)),
      timestamp(std::move(t)),
      batch_position(position),
      batch_size(size) {}

Record::~Record() = default;

Record::Record(Record&& other) = default;

Record& Record::operator=(Record&& other) = default;

}  // namespace cloud_provider_firebase
