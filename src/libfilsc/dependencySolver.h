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


/// <summary>
/// Provides a mechanism to sort a set of items which have dependencies between them.
/// The result is sorted so that the dependencies of an item are allways in a previous position
/// int he resulting vector.
/// </summary>
/// <remarks>There MUST NOT be circular references. If there are, the function may never return.</remarks>
/// <param name="items">Initial set of items</param>
/// <param name="depFN">Function which returns the dependencies of a particular item.</param>
/// <returns>The dependency-sorted array. It includes the initial set of items ('items' parameter),
/// and any dependencies discovered calling 'depFN'</returns>
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
