/// <summary>
/// Tests FIL-S build system.
/// </summary>

#include "libfilsc_test_pch.h"
#include "builder_internal.h"
#include "compileError.h"

/// <summary>
/// Tests 'buildModule' function
/// </summary>
TEST(Builder, buildModule)
{
    //TODO: This test is just a 'placerholder' which calls the build system
    //Just to force to link it.
    //I'm not expecting it to work without modifications.
    auto r = buildModule(".", fs::current_path().u8string());

    ASSERT_TRUE(r.ok());
}
