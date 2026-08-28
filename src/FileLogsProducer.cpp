/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2023, Janusz Wolak
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

#include <fmt/format.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "FileLogsProducer.h"

void equinox::FileLogsProducer::setupFile(const std::string& logFileName, std::size_t maxLogFileSizeBytes, std::size_t maxLogFiles) {
    mLogFileName_ = logFileName;
    mMaxLogFileSizeBytes_ = maxLogFileSizeBytes;
    mMaxLogFiles_ = maxLogFiles;
    mNextRotationIndex_ = 1U;

    if (mFdLogFile_.is_open()) {
        try {
            mFdLogFile_.close();
        } catch (std::ofstream::failure& ex) {                                                                    // LCOV_EXCL_LINE
            std::cerr << fmt::format("[EquinoxLogger] Exception when closing file: {}", ex.what()) << std::endl;  // LCOV_EXCL_LINE
        }  // LCOV_EXCL_LINE
    }

    openLogFileAppend();

    if (!mFdLogFile_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + mLogFileName_);
    }
}

void equinox::FileLogsProducer::openLogFileAppend() {
    if (mFdLogFile_.is_open()) {
        return;
    }

    mFdLogFile_.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        mFdLogFile_.open(mLogFileName_, std::ofstream::out | std::ofstream::app);
    } catch (std::ofstream::failure& ex) {
        std::cerr << fmt::format("[EquinoxLogger] Failed to open log file: {} - {}", mLogFileName_, ex.what()) << std::endl;
        mFdLogFile_.clear();
    }
}

void equinox::FileLogsProducer::openLogFileTruncate() {
    if (mFdLogFile_.is_open()) {
        return;
    }

    mFdLogFile_.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        mFdLogFile_.open(mLogFileName_, std::ofstream::out | std::ofstream::trunc);
    } catch (std::ofstream::failure& ex) {
        std::cerr << fmt::format("[EquinoxLogger] Failed to open log file: {} - {}", mLogFileName_, ex.what()) << std::endl;
        mFdLogFile_.clear();
    }
}

bool equinox::FileLogsProducer::isRotationEnabled() const {
    return (mMaxLogFileSizeBytes_ > 0U) && (mMaxLogFiles_ > 0U);
}

std::string equinox::FileLogsProducer::buildRotatedFileName(std::size_t index) const {
    std::filesystem::path basePath(mLogFileName_);
    std::string rotatedName = basePath.stem().string() + "_" + std::to_string(index) + basePath.extension().string();
    return (basePath.parent_path() / rotatedName).string();
}

void equinox::FileLogsProducer::rotateIfNeeded() {
    if (!isRotationEnabled()) {
        return;
    }

    std::error_code errorCode;
    std::uintmax_t fileSize = std::filesystem::file_size(mLogFileName_, errorCode);
    if (errorCode) {
        std::cerr << fmt::format("[EquinoxLogger] Failed to check file size: {}", errorCode.message()) << std::endl;
        return;
    }

    if (fileSize < mMaxLogFileSizeBytes_) {
        return;
    }

    try {
        mFdLogFile_.close();
    } catch (std::ofstream::failure& ex) {                                                                                    // LCOV_EXCL_LINE
        std::cerr << fmt::format("[EquinoxLogger] Exception when closing file during rotation: {}", ex.what()) << std::endl;  // LCOV_EXCL_LINE
    }  // LCOV_EXCL_LINE

    std::string rotatedFileName = buildRotatedFileName(mNextRotationIndex_);
    std::filesystem::remove(rotatedFileName, errorCode);
    errorCode.clear();
    std::filesystem::rename(mLogFileName_, rotatedFileName, errorCode);

    if (errorCode) {
        std::cerr << fmt::format("[EquinoxLogger] Failed to rotate log file: {}", errorCode.message()) << std::endl;
        openLogFileAppend();
        return;
    }

    if (mMaxLogFiles_ > 0U) {
        mNextRotationIndex_ = (mNextRotationIndex_ % mMaxLogFiles_) + 1U;
    }

    openLogFileTruncate();
}

void equinox::FileLogsProducer::logMessage(const std::string& messageToLog) {
    std::lock_guard<std::mutex> lock(mMessageBufferAccessLock_);

    if (!mFdLogFile_.is_open()) {
        std::cerr << "[EquinoxLogger] Log file is not open, cannot write message" << std::endl;
        return;
    }

    thread_local std::string buffer;
    buffer.clear();
    buffer = mTimestampProducer->getTimestamp() + mTimestampProducer->getTimestampInUs() + messageToLog;

    try {
        mFdLogFile_ << buffer << std::endl;
        mFdLogFile_.flush();
    } catch (std::ofstream::failure& ex) {                                                                    // LCOV_EXCL_LINE
        std::cerr << fmt::format("[EquinoxLogger] Failed to write to log file: {}", ex.what()) << std::endl;  // LCOV_EXCL_LINE
        return;                                                                                               // LCOV_EXCL_LINE
    }  // LCOV_EXCL_LINE

    rotateIfNeeded();
}

void equinox::FileLogsProducer::flush() {
    std::lock_guard<std::mutex> lock(mMessageBufferAccessLock_);
    if (mFdLogFile_.is_open()) {
        try {
            mFdLogFile_.flush();
        } catch (std::ofstream::failure& ex) {                                                                 // LCOV_EXCL_LINE
            std::cerr << fmt::format("[EquinoxLogger] Failed to flush log file: {}", ex.what()) << std::endl;  // LCOV_EXCL_LINE
        }  // LCOV_EXCL_LINE
    }
}

// for testing purposes only
std::ofstream& equinox::FileLogsProducer::GetLogFileStream() {
    return mFdLogFile_;
}

std::string& equinox::FileLogsProducer::GetLogFileName() {
    return mLogFileName_;
}

std::size_t& equinox::FileLogsProducer::GetMaxLogFileSizeBytes() {
    return mMaxLogFileSizeBytes_;
}

std::size_t& equinox::FileLogsProducer::GetMaxLogFiles() {
    return mMaxLogFiles_;
}

std::size_t& equinox::FileLogsProducer::GetNextRotationIndex() {
    return mNextRotationIndex_;
}
