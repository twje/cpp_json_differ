#pragma once

// Includes
//--------------------------------------------------------------------------
// Application
#include "JsonPath.h"

// Third Party
#include "json.hpp"

// Type Aliases
//--------------------------------------------------------------------------
using json = nlohmann::json;

//--------------------------------------------------------------------------
class IJsonDiffProcessor
{
public:
	virtual ~IJsonDiffProcessor() = default;
	virtual void OnDiffAdd(const JsonPath& path, const json& addedValue) = 0;
	virtual void OnDiffRemove(const JsonPath& path, const json& removedValue) = 0;
	virtual void OnDiffReplace(const JsonPath& path, const json& oldValue, const json& newValue) = 0;
	virtual void OnDiffReorder(const JsonPath& path, size_t index1, const json& value1, size_t index2, const json& value2) = 0;
};
