#include "pch.h"
#include "builder.h"
#include "compileError.h"
#include "ast.h"

using namespace std;

typedef map < string, Ref<AstNode>>	AstNodeMap;

vector<string>	getModuleSources(const std::string& modulePath);
BuildResult		parseSources(const vector<string>& sources, AstNodeMap* parsedSources);
BuildResult		getModuleDependencies(
	const std::string& modulePath, 
	const AstNodeMap& parsedSources,
	map<string, string>* dependencies
);
BuildResult		buildDependencies(
	const map<string, string>& dependencies, 
	AstNodeMap* dependenciesAST
);
BuildResult		finalModuleBuild(
	const AstNodeMap& parsedSources,
	const AstNodeMap& dependenciesAST
);


/// <summary>
/// Builds a module, from its sources on the filesystem.
/// </summary>
/// <param name="moduleDir"></param>
/// <returns></returns>
BuildResult buildModule(const std::string& modulePath)
{
	//auto result = buildDependencyTree(modulePath);

	//if (result.ok)

	///
	vector<string>	sources = getModuleSources(modulePath);

	if (sources.empty())
		return BuildResult::error("No FIL-S sources found in directory: %s", modulePath.c_str());

	AstNodeMap		parsedSources;

	auto r = parseSources(sources, &parsedSources);
	if (!r.ok())
		return r;

	map<string, string> dependencies;
	r = getModuleDependencies(modulePath, parsedSources, &dependencies);
	if (!r.ok())
		return r;

	AstNodeMap		dependenciesAST;
	r = buildDependencies(dependencies, &dependenciesAST);
	if (!r.ok())
		return r;

	return finalModuleBuild(parsedSources, dependenciesAST);
}
