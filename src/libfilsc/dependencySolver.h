#pragma once

#include <vector>
#include <map>
#include <set>
#include <functional>


template <class Type>
bool solveDependencies(
    Type item,
    std::map<Type, int>& solvedMap,
    std::set<Type>& circularGuard,
    std::function < std::set<Type>(const Type&)> depFN
)
{
    //Already solved.
    if (solvedMap.count(item) > 0)
        return true;

    if (circularGuard.count(item) > 0)
        return false;	//Circular reference detected.
    else
        circularGuard.insert(item);

    auto dependencies = depFN(item);
    int level = 0;

    for (auto& dep : dependencies)
    {
        if (!solveDependencies(dep, solvedMap, circularGuard, depFN))
            return false;	//Circular reference detected.

        auto solvedIt = solvedMap.find(dep);
        level = max(level, solvedIt->second + 1);
    }//for.

    solvedMap[item] = level;

    return true;
}

//TODO: Provide more information in comments, if this code is kept.
/// <summary>
/// Sorts a set of items taking into account its dependencies.
/// </summary>
template <class Type>
std::vector<Type> dependencySort(
    const std::vector<Type>& items,
    std::function < std::set<Type>(const Type&)> depFN)
{
    typedef std::vector<Type>		ItemVector;
    typedef std::set<Type>			ItemSet;
    typedef std::vector<ItemVector>	ItemLevelsContainer;
    typedef std::map<Type, int>		SolvedDependenciesMap;

    SolvedDependenciesMap	solved;

    for (auto& item : items)
    {
        if (solved.count(item) == 0)
        {
            ItemSet	circularGuard;

            if (!solveDependencies(item, solved, circularGuard, depFN))
                return std::vector<Type>();		//Circular dependency found, return empty
        }
    }

    //Sort by level.
    ItemLevelsContainer		levels;
    for (auto& itemEntry : solved)
    {
        if (itemEntry.second >= (int)levels.size())
            levels.resize(itemEntry.second + 1);

        levels[itemEntry.second].push_back(itemEntry.first);
    }
    solved.clear();

    //Flatten levels. Yields a vector sorted by level.
    ItemVector			result;
    for (auto& level : levels)
        result.insert(result.end(), level.begin(), level.end());

    return result;
}
