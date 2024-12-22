#pragma once

// Includes
//--------------------------------------------------------------------------
// Application
#include "JsonPath.h"

// Third Party
#include "json.hpp"

// System
#include <string_view>
#include <sstream>

// Type Aliases
//--------------------------------------------------------------------------
using json = nlohmann::json;

//--------------------------------------------------------------------------
class JsonNavigator
{
public:
    explicit JsonNavigator(const json& data)
        : mData(data)
        , mCurrent(&mData)
    { }

    const json* Navigate(const JsonPath& jsonPath) const
    {
        ResetToRoot();
        for (const auto& token : jsonPath.Tokens())
        {
            if (!NavigateToken(token))
            {
                return nullptr;
            }
        }
        return mCurrent;
    }

private:
    void ResetToRoot() const
    {
        mCurrent = &mData;
    }

    bool NavigateToken(const std::string& token) const
    {
        if (mCurrent->is_object())
        {
            return NavigateObject(token);
        }
        else if (mCurrent->is_array())
        {
            return NavigateArray(token);
        }
        return false;
    }

    bool NavigateObject(const std::string& key) const
    {
        if (mCurrent->contains(key))
        {
            mCurrent = &(*mCurrent)[key];
            return true;
        }
        return false;
    }

    bool NavigateArray(const std::string& indexStr) const
    {
        try
        {
            size_t index = std::stoul(indexStr);
            if (index < mCurrent->size())
            {
                mCurrent = &(*mCurrent)[index];
                return true;
            }
        }
        catch (const std::invalid_argument&)
        {
            // Invalid index
        }
        return false;
    }

    const json& mData;
    mutable const json* mCurrent; // `mutable` because state is modified in const methods
};