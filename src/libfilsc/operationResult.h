#pragma once

#include "compileError.h"
#include <vector>


/// <summary>
/// 'OperationResult' is used to return results from several build/compile operations.
/// It contains either:
///   * A ssucessful result, which type is the template parameter, in order to 
///   use this class for several distinct operations.
///   * A list of 'CompileError', in case of a failed operation.
/// </summary>
template <class ResultType>
class OperationResult
{
public:
	typedef std::vector<CompileError>	ErrorList;

	/// <summary>Successful result constructor</summary>
	/// <param name="r">Result item, which will copied into 'result' field.</param>
	OperationResult(ResultType r) : result(r)
	{}

	/// <summary>Single error constructor.</summary>
	/// <param name="error"></param>
	OperationResult(const CompileError& error)
		: errors({ error })
	{
	}

	/// <summary>Multiple errors constructor</summary>
	/// <param name="errors_"></param>
	OperationResult(const ErrorList& errors_)
		: errors(errors_)
	{
	}

	/// <summary>Returns if the operation has been successful (true) or has some errors (false)</summary>
	bool ok()const
	{
		return errors.empty();
	}

	/// <summary>
	/// Combine two results.
	/// If both operations are sucessful, it yields the second one (the parameter)
	/// </summary>
	/// <returns>The return type is the result type of the parameter.</returns>
	template <class O>
	OperationResult <O> combineWith(const OperationResult <O>& r2)const
	{
		if (!ok())
		{
			auto combinedErrors = this->errors;
			combinedErrors.insert(combinedErrors.end(), r2.errors.begin(), r2.errors.end());

			return OperationResult<O>(combinedErrors);
		}
		else
			return r2;
	}

	const ResultType	result;
	const ErrorList		errors;

};
