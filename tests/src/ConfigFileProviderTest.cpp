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

#include "ConfigFileProvider.h"

namespace config_file_provider_test {
    using namespace ::testing;
    using namespace equinox;

    class ConfigFileProviderTestable : public ConfigFileProvider {
       public:
        ConfigFileProviderTestable() : ConfigFileProvider() {}

        using ConfigFileProvider::loadConfig;
        using ConfigFileProvider::trim;
    };

    class ConfigFileProviderTest : public Test {
       public:
        ConfigFileProviderTest() {}

        ConfigFileProviderTestable config_file_provider;
    };

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_Load_Config_Map_An_Exception_Thrown_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_Loaded_Config_Map_Is_Empty_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Log_Level_From_Config_File_But_It_Fails_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Log_Prefix_From_Config_File_But_It_Fails_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Logs_Output_Sink_From_Config_File_But_It_Fails_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Log_File_Name_From_Config_File_But_It_Fails_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Max_Log_File_Size_Bytes_From_Config_File_But_It_Fails_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Max_Log_Files_From_Config_File_But_It_Fails_And_Nullopt_Returned) {}

    TEST_F(ConfigFileProviderTest, Load_Config_From_File_Successfull_And_Configuration_Returned) {}
}  // namespace config_file_provider_test