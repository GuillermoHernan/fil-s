// libfilsc_test.cpp : Defines the entry point for the console application.
//

/// <summary>
/// Test program for 'libfilsc' library.
/// </summary>

#include "libfilsc_test_pch.h"


int main (int argc, char **argv)
{
	//::testing::GTEST_FLAG(filter) = "SemanticAnalysis.semInOrderWalk";
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

