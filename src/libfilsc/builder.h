/// <summary>
/// Builder: Builds FIL-S modules from its sources and dependencies.
/// </summary>

#pragma once
#include <string>

class BuildResult;

BuildResult buildModule(const std::string& modulePath);

/// <summary>
/// Result from a build module operation
/// </summary>
class BuildResult
{
public:
	static BuildResult error(const char* format, ...);

	bool ok()const;
};