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

#ifndef INCLUDE_ASYNCLOGQUEUE_H_
#define INCLUDE_ASYNCLOGQUEUE_H_

#include <condition_variable>
#include <cstddef>
#include <cstdint>

#include <deque>
#include <mutex>
#include <string>
#include <vector>

#include "IAsyncLogQueue.h"

namespace equinox {
    class AsyncLogQueue : public IAsyncLogQueue {
       public:
        explicit AsyncLogQueue(size_t queue_max_size);
        ~AsyncLogQueue();
        void enqueue(const std::string& log_message) override;
        bool dequeue(std::vector<std::string>& out, size_t max_batch_size, uint32_t timeout_ms) override;
        void stop() override;

       protected:
        std::deque<std::string>& getLogMessagesQueue();
        void setStopRequested(bool stopRequested);
        bool getStopRequested();

       private:
        size_t mQueueMaxSize_;
        std::deque<std::string> mLogMessagesQueue_;
        std::mutex mLogMessagesQueueMutex_;
        std::condition_variable mDataInQueueAvailableConditionVariable_;
        bool mStopRequested_;
    };
}  // namespace equinox

#endif /* INCLUDE_ASYNCLOGQUEUE_H_ */
