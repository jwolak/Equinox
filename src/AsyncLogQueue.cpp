/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Janusz Wolak
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <chrono>

#include "AsyncLogQueue.h"

equinox::AsyncLogQueue::AsyncLogQueue(size_t queue_max_size)
    : mQueueMaxSize_(queue_max_size), mLogMessagesQueueMutex_{}, mDataInQueueAvailableConditionVariable_{}, mStopRequested_(false) {}

equinox::AsyncLogQueue::~AsyncLogQueue() = default;

void equinox::AsyncLogQueue::enqueue(const std::string& log_message) {
    std::unique_lock<std::mutex> lock(mLogMessagesQueueMutex_);
    if (mLogMessagesQueue_.size() >= mQueueMaxSize_) {
        mLogMessagesQueue_.pop_front();  // Remove the oldest log message to make room for the new one
    }
    mLogMessagesQueue_.push_back(log_message);
    lock.unlock();
    mDataInQueueAvailableConditionVariable_.notify_one();
}

bool equinox::AsyncLogQueue::dequeue(std::vector<std::string>& out, size_t max_batch_size, uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lock(mLogMessagesQueueMutex_);
    if (!mDataInQueueAvailableConditionVariable_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                                          [this]() { return !mLogMessagesQueue_.empty() || mStopRequested_; })) {
        return false;
    }

    if (mStopRequested_ && mLogMessagesQueue_.empty()) {
        return false;
    }

    size_t batch_size = std::min(max_batch_size, mLogMessagesQueue_.size());
    out.reserve(batch_size);
    for (size_t i = 0; i < batch_size; ++i) {
        out.push_back(std::move(mLogMessagesQueue_.front()));
        mLogMessagesQueue_.pop_front();
    }

    return true;
}

void equinox::AsyncLogQueue::stop() {
    std::unique_lock<std::mutex> lock(mLogMessagesQueueMutex_);
    mStopRequested_ = true;
    lock.unlock();
    mDataInQueueAvailableConditionVariable_.notify_all();
}

std::deque<std::string>& equinox::AsyncLogQueue::getLogMessagesQueue() {
    std::lock_guard<std::mutex> lock(mLogMessagesQueueMutex_);
    return mLogMessagesQueue_;
}

void equinox::AsyncLogQueue::setStopRequested(bool stopRequested) {
    std::lock_guard<std::mutex> lock(mLogMessagesQueueMutex_);
    mStopRequested_ = stopRequested;
}

bool equinox::AsyncLogQueue::getStopRequested() {
    std::lock_guard<std::mutex> lock(mLogMessagesQueueMutex_);
    return mStopRequested_;
}