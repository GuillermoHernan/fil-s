/// <summary>
/// Builder: Builds FIL-S modules from its sources and dependencies.
/// </summary>

#pragma once
#include <string>
#include "operationResult.h"

typedef OperationResult<bool> BuildResult;

BuildResult buildModule(const std::string& modulePath, const std::string& builderPath);
