// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/vsync_waiter.h"

#include "flutter/fml/task_runner.h"
#include "flutter/fml/trace_event.h"

namespace shell {

#if defined(OS_FUCHSIA)
// In general, traces on Fuchsia are recorded across the whole system.
// Because of this, emitting a "VSYNC" event per flutter process is
// undesirable, as the events will collide with each other.  We
// instead let another area of the system emit them.
static constexpr const char* kVsyncTraceName = "vsync callback";
#else   // defined(OS_FUCHSIA)
// Note: The tag name must be "VSYNC" (it is special) so that the
// "Highlight Vsync" checkbox in the timeline can be enabled.
static constexpr const char* kVsyncTraceName = "VSYNC";
#endif  // defined(OS_FUCHSIA)

static constexpr const char* kVsyncFlowName = "VsyncFlow";

VsyncWaiter::VsyncWaiter(blink::TaskRunners task_runners)
    : task_runners_(std::move(task_runners)) {}

VsyncWaiter::~VsyncWaiter() = default;

void VsyncWaiter::AsyncWaitForVsync(Callback callback) {
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback_ = std::move(callback);
  }
  AwaitVSync();
}

void VsyncWaiter::FireCallback(fml::TimePoint frame_start_time,
                               fml::TimePoint frame_target_time) {
  Callback callback;

  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback = std::move(callback_);
  }

  if (!callback) {
    return;
  }

  auto flow_identifier = fml::tracing::TraceNonce();

  // The base trace ensures that flows have a root to begin from if one does not
  // exist. The trace viewer will ignore traces that have no base event trace.
  // While all our message loops insert a base trace trace
  // (MessageLoop::RunExpiredTasks), embedders may not.
  TRACE_EVENT0("flutter", "VsyncFireCallback");
  TRACE_FLOW_BEGIN("flutter", kVsyncFlowName, flow_identifier);

  task_runners_.GetUITaskRunner()->PostTaskForTime(
      [callback, flow_identifier, frame_start_time, frame_target_time]() {
        FML_TRACE_EVENT("flutter", kVsyncTraceName, "StartTime",
                        frame_start_time, "TargetTime", frame_target_time);
        TRACE_FLOW_END("flutter", kVsyncFlowName, flow_identifier);
        callback(frame_start_time, frame_target_time);
      },
      frame_start_time);
}

float VsyncWaiter::GetDisplayRefreshRate() const {
  return kUnknownRefreshRateFPS;
}

}  // namespace shell
