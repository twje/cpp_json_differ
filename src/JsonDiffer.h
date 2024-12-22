#pragma once

// Includes
//--------------------------------------------------------------------------
// Application
#include "JsonPath.h"
#include "JsonNavigator.h"
#include "IJsonDiffProcessor.h"

// Third Party
#include "json.hpp"

// System
#include <string>
#include <unordered_set>
#include <cassert>
#include <stdexcept>
#include <vector>

// Type Aliases
//--------------------------------------------------------------------------
using json = nlohmann::json;

//--------------------------------------------------------------------------
enum class JsonDiffType
{
	Add,
	Remove,
	Replace,
	Reorder,
	Unknown
};

//--------------------------------------------------------------------------
struct JsonDiffOperation
{
public:
	explicit JsonDiffOperation(const json& operation)
		: mDiffType(ExtractDiffType(operation))
		, mPath(ExtractPath(operation))
		, mValue(ExtractValue(operation))
	{ }

	JsonDiffOperation(JsonDiffType diffType, const JsonPath& path, const json* value)
		: mDiffType(diffType)
		, mPath(path)
		, mValue(value)
	{ }

	JsonDiffType mDiffType;
	JsonPath mPath;
	const json* mValue;

private:
	static JsonDiffType ExtractDiffType(const json& operation)
	{
		if (operation.contains("op"))
		{
			const std::string op = operation.at("op").get<std::string>();
			if (op == "add")
			{
				return JsonDiffType::Add;
			}
			else if (op == "remove")
			{
				return JsonDiffType::Remove;
			}
			else if (op == "replace")
			{
				return JsonDiffType::Replace;
			}
			else if (op == "reorder")
			{
				return JsonDiffType::Reorder;
			}
		}

		return JsonDiffType::Unknown;
	}

	static JsonPath ExtractPath(const json& operation)
	{
		return JsonPath(operation.at("path").get<std::string>());
	}

	static const json* ExtractValue(const json& operation)
	{
		return operation.contains("value") ? &operation.at("value") : nullptr;
	}
};

//--------------------------------------------------------------------------
class JsonDiffer
{
public:
	JsonDiffer(const json& oldJson, const json& newJson)
		: mOldJson(oldJson)
		, mNewJson(newJson)
		, mOldNavigator(oldJson)
		, mNewNavigator(newJson)
	{ }

	void ProcessDiff(IJsonDiffProcessor& processor)
	{
		const json patch = json::diff(mOldJson, mNewJson);
		std::unordered_set<JsonPath> processedPaths;

		for (const auto& patchOperation : patch)
		{
			JsonDiffOperation operation(patchOperation);
			assert(operation.mDiffType != JsonDiffType::Unknown);

			if (processedPaths.count(operation.mPath))
			{
				continue;
			}

			switch (operation.mDiffType)
			{
				case JsonDiffType::Replace:
				{
					HandleReplace(processor, operation, patch, processedPaths);
					break;
				}
				case JsonDiffType::Add:
				{
					HandleAdd(processor, operation);
					break;
				}
				case JsonDiffType::Remove:
				{
					HandleRemove(processor, operation);
					break;
				}
			}
		}
	}

private:
	void HandleReplace(IJsonDiffProcessor& processor, const JsonDiffOperation& operation, const json& patch,
		std::unordered_set<JsonPath>& processedPaths)
	{
		// Only arrays can be reordered
		const bool isArray = mNewNavigator.Navigate(operation.mPath.Parent())->is_array();
		if (!isArray)
		{
			ProcessSimpleReplace(processor, operation);
			return;
		}

		if (DetectReorder(processor, operation, patch, processedPaths))
		{
			return;
		}

		// Fallback to simple replace if no reorder detected
		ProcessSimpleReplace(processor, operation);
	}

	void ProcessSimpleReplace(IJsonDiffProcessor& processor, const JsonDiffOperation& operation)
	{
		const json& newValue = *operation.mValue;
		const json& oldValue = *mOldNavigator.Navigate(operation.mPath);
		processor.OnDiffReplace(operation.mPath, oldValue, newValue);
	}

	bool DetectReorder(IJsonDiffProcessor& processor, const JsonDiffOperation& operation, const json& patch,
		std::unordered_set<JsonPath>& processedPaths)
	{
		for (const auto& siblingOperation : patch)
		{
			JsonDiffOperation sibling(siblingOperation);
			if (sibling.mDiffType != JsonDiffType::Replace || !operation.mPath.AreSiblings(sibling.mPath))
			{
				continue;
			}

			const json& newValue = *operation.mValue;
			const json& oldValue = *mOldNavigator.Navigate(operation.mPath);

			const json& siblingNewValue = *sibling.mValue;
			const json& siblingOldValue = *mOldNavigator.Navigate(sibling.mPath);

			if (newValue == siblingOldValue && oldValue == siblingNewValue)
			{
				size_t index = std::stoul(operation.mPath.Tokens().back());
				size_t siblingIndex = std::stoul(sibling.mPath.Tokens().back());

				processor.OnDiffReorder(operation.mPath, index, newValue, siblingIndex, siblingNewValue);
				processedPaths.insert(sibling.mPath);
				return true;
			}
		}
		return false;
	}

	void HandleAdd(IJsonDiffProcessor& processor, const JsonDiffOperation& operation)
	{
		processor.OnDiffAdd(operation.mPath, *operation.mValue);
	}

	void HandleRemove(IJsonDiffProcessor& processor, const JsonDiffOperation& operation)
	{
		const json& removedValue = *mOldNavigator.Navigate(operation.mPath);
		processor.OnDiffRemove(operation.mPath, removedValue);
	}

	const json& mOldJson;
	const json& mNewJson;
	JsonNavigator mOldNavigator;
	JsonNavigator mNewNavigator;
};