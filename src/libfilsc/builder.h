/// <summary>
/// Builder: Builds FIL-S modules from its sources and dependencies.
/// </summary>

#pragma once
#include <string>
#include "operationResult.h"

/// <summary>
/// Stores builder configuration.
/// </summary>
struct BuilderConfig
{
    std::string      BasePath;
    std::string      RuntimePath;
    std::string      PlatformName;
    std::string      PlatformPath;
};

typedef OperationResult<bool> BuildResult;

BuildResult buildModule(const std::string& modulePath, const BuilderConfig& cfg);
