// Includes
//--------------------------------------------------------------------------
// Application
#include "JsonDiffer.h"
#include "IJsonDiffProcessor.h"

// System
#include <iostream>

//--------------------------------------------------------------------------
class EmailConfigChangeGenerator : public IJsonDiffProcessor
{
public:
	virtual void OnDiffAdd(const JsonPath& path, const json& addedValue) override
	{
		std::cout << "Add: " << path << " = " << addedValue << std::endl;
	}

	virtual void OnDiffRemove(const JsonPath& path, const json& removedValue) override
	{
		std::cout << "Remove: " << path << " = " << removedValue << std::endl;
	}

	virtual void OnDiffReplace(const JsonPath& path, const json& oldValue, const json& newValue) override
	{
		std::cout << "Replace: " << path << " = " << oldValue << " -> " << newValue << std::endl;
	}

	virtual void OnDiffReorder(const JsonPath& path, size_t index1, const json& value1, size_t index2, const json& value2) override
	{
		std::cout << "Reorder: " << path << " [" << index1 << "] = " << value1
			<< " <-> [" << index2 << "] = " << value2 << std::endl;
	}
};

//--------------------------------------------------------------------------
int main()
{
	// Create old and new JSON objects for testing
	json oldJson = R"({
        "name": "Alice",
        "age": 30,
        "skills": ["C++", "Python"],
        "projects": {
            "active": ["Project A", "Project B"],
            "completed": ["Project C"]
        }
    })"_json;

	json newJson = R"({
        "name": "Alice",
        "age": 31,
        "skills": ["Python", "C++"],
        "projects": {
            "active": ["Project A", "Project D"],
            "completed": []
        },
        "location": "New York"
    })"_json;
	
	EmailConfigChangeGenerator processor;
	
	JsonDiffer diff(oldJson, newJson);
	diff.ProcessDiff(processor);

	/*
		Output:
			Replace: /age = 30 -> 31
			Replace: /projects/active/1 = "Project B" -> "Project D"
			Remove: /projects/completed/0 = "Project C"
			Reorder: /skills/0 [0] = "Python" <-> [1] = "C++"
			Add: /location = "New York"
	*/

	return 0;
}