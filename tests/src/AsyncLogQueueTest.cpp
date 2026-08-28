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

#include <gtest/gtest.h>

#include <thread>

#include "AsyncLogQueue.h"

namespace async_log_queue_test {
    namespace {
        constexpr size_t kTestQueueMaxSize = 10;
        constexpr const char* testMessage = "Test log message";
    }  // namespace

    class AsyncLogQueueForTests : public ::equinox::AsyncLogQueue {
       public:
        explicit AsyncLogQueueForTests(size_t queue_max_size) : AsyncLogQueue(queue_max_size) {}

        std::deque<std::string>& getInternalQueue() {
            return getLogMessagesQueue();
        }
        void setStopRequested(bool stopRequested) {
            AsyncLogQueue::setStopRequested(stopRequested);
        }
        bool getStopRequested() {
            return AsyncLogQueue::getStopRequested();
        }
    };

    class AsyncLogQueueTest : public ::testing::Test {
       public:
        AsyncLogQueueTest() : asyncLogQueue{kTestQueueMaxSize} {}

        AsyncLogQueueForTests asyncLogQueue;
    };

    TEST_F(AsyncLogQueueTest, Queue_Is_Full_And_Oldest_Message_Is_Removed) {
        for (size_t i = 0; i < kTestQueueMaxSize; ++i) {
            asyncLogQueue.enqueue("Log message " + std::to_string(i));
        }
        ASSERT_EQ(asyncLogQueue.getInternalQueue().size(), kTestQueueMaxSize);
        asyncLogQueue.enqueue(testMessage);

        ASSERT_EQ(asyncLogQueue.getInternalQueue().size(), kTestQueueMaxSize);
    }

    TEST_F(AsyncLogQueueTest, Queue_Is_Empty_And_After_Enqueue_Size_Is_One) {
        ASSERT_EQ(asyncLogQueue.getInternalQueue().size(), 0);
        asyncLogQueue.enqueue(testMessage);

        ASSERT_EQ(asyncLogQueue.getInternalQueue().size(), 1);
    }

    TEST_F(AsyncLogQueueTest, Run_Enqueue_From_Two_Thread_And_Queue_Size_Is_Correct) {
        std::thread t1([this]() {
            for (size_t i = 0; i < kTestQueueMaxSize / 2; ++i) {
                asyncLogQueue.enqueue("Thread 1 - Log message " + std::to_string(i));
            }
        });

        std::thread t2([this]() {
            for (size_t i = 0; i < kTestQueueMaxSize / 2; ++i) {
                asyncLogQueue.enqueue("Thread 2 - Log message " + std::to_string(i));
            }
        });

        t1.join();
        t2.join();

        ASSERT_EQ(asyncLogQueue.getInternalQueue().size(), kTestQueueMaxSize);
    }

    TEST_F(AsyncLogQueueTest, Run_Enqueue_From_Two_Thread_And_Messages_Are_Correctly_Added_To_Queue) {
        std::thread t1([this]() {
            for (size_t i = 0; i < kTestQueueMaxSize / 2; ++i) {
                asyncLogQueue.enqueue("Thread 1 - Log message " + std::to_string(i));
            }
        });

        std::thread t2([this]() {
            for (size_t i = 0; i < kTestQueueMaxSize / 2; ++i) {
                asyncLogQueue.enqueue("Thread 2 - Log message " + std::to_string(i));
            }
        });

        t1.join();
        t2.join();

        const auto& queue = asyncLogQueue.getInternalQueue();
        for (size_t i = 0; i < kTestQueueMaxSize / 2; ++i) {
            ASSERT_TRUE(std::find(queue.begin(), queue.end(), "Thread 1 - Log message " + std::to_string(i)) != queue.end());
            ASSERT_TRUE(std::find(queue.begin(), queue.end(), "Thread 2 - Log message " + std::to_string(i)) != queue.end());
        }
    }

    TEST_F(AsyncLogQueueTest, Try_Dequeue_From_Empty_Queue_And_Return_False) {
        std::vector<std::string> out;

        ASSERT_FALSE(asyncLogQueue.dequeue(out, 5, 100));
    }

    TEST_F(AsyncLogQueueTest, Try_Dequeue_When_Stop_Is_Requested_And_Return_False) {
        asyncLogQueue.setStopRequested(true);
        std::vector<std::string> out;

        ASSERT_FALSE(asyncLogQueue.dequeue(out, 5, 100));
    }

    TEST_F(AsyncLogQueueTest, Enqueue_And_Dequeue_Messages_And_Check_If_Dequeued_Messages_Are_Correct) {
        std::vector<std::string> messagesToEnqueue = {"Message 1", "Message 2", "Message 3"};
        for (const auto& msg : messagesToEnqueue) {
            asyncLogQueue.getInternalQueue().push_back(msg);
        }

        std::vector<std::string> out;
        ASSERT_TRUE(asyncLogQueue.dequeue(out, 5, 100));
        ASSERT_EQ(out.size(), messagesToEnqueue.size());
        for (const auto& msg : messagesToEnqueue) {
            ASSERT_TRUE(std::find(out.begin(), out.end(), msg) != out.end());
        }
    }

    TEST_F(AsyncLogQueueTest, Enqueue_More_Messages_Than_Max_Batch_Size_And_Check_If_Dequeued_Messages_Are_Correct) {
        std::vector<std::string> messagesToEnqueue = {"Message 1", "Message 2", "Message 3", "Message 4", "Message 5"};
        for (const auto& msg : messagesToEnqueue) {
            asyncLogQueue.getInternalQueue().push_back(msg);
        }

        std::vector<std::string> out;
        ASSERT_TRUE(asyncLogQueue.dequeue(out, 3, 100));
        ASSERT_EQ(out.size(), 3);
        for (size_t i = 0; i < 3; ++i) {
            ASSERT_TRUE(std::find(out.begin(), out.end(), messagesToEnqueue[i]) != out.end());
        }
    }

    TEST_F(AsyncLogQueueTest, Dequeue_From_Two_Threads_And_Check_If_Messages_Are_Correctly_Dequeued) {
        std::vector<std::string> messagesToEnqueue = {"Message 1", "Message 2", "Message 3", "Message 4", "Message 5"};
        for (const auto& msg : messagesToEnqueue) {
            asyncLogQueue.getInternalQueue().push_back(msg);
        }

        std::vector<std::string> out1, out2;
        std::thread t1([this, &out1]() {
            bool result = asyncLogQueue.dequeue(out1, 3, 100);
            ASSERT_TRUE(result);
        });

        std::thread t2([this, &out2]() {
            bool result = asyncLogQueue.dequeue(out2, 3, 100);
            ASSERT_TRUE(result);
        });

        t1.join();
        t2.join();

        std::vector<std::string> allDequeuedMessages;
        allDequeuedMessages.insert(allDequeuedMessages.end(), out1.begin(), out1.end());
        allDequeuedMessages.insert(allDequeuedMessages.end(), out2.begin(), out2.end());

        for (const auto& msg : messagesToEnqueue) {
            ASSERT_TRUE(std::find(allDequeuedMessages.begin(), allDequeuedMessages.end(), msg) != allDequeuedMessages.end());
        }
    }

    TEST_F(AsyncLogQueueTest, Dequeue_All_Messages_And_Check_If_Queue_Is_Empty) {
        std::vector<std::string> messagesToEnqueue = {"Message 1", "Message 2", "Message 3"};
        for (const auto& msg : messagesToEnqueue) {
            asyncLogQueue.getInternalQueue().push_back(msg);
        }

        std::vector<std::string> out;
        asyncLogQueue.dequeue(out, 5, 100);

        ASSERT_TRUE(asyncLogQueue.getInternalQueue().empty());
    }

    TEST_F(AsyncLogQueueTest, Stop_Queue_And_Check_If_Dequeue_Returns_False) {
        asyncLogQueue.stop();

        ASSERT_TRUE(asyncLogQueue.getStopRequested());
    }

    TEST_F(AsyncLogQueueTest, Check_If_Stop_Request_Set_To_False_By_Default) {
        ASSERT_FALSE(asyncLogQueue.getStopRequested());
    }
}  // namespace async_log_queue_test