#pragma once

// Includes
//--------------------------------------------------------------------------
#include <string>
#include <sstream>
#include <vector>
#include <ostream>
#include <stdexcept>

//--------------------------------------------------------------------------
class JsonPath
{
public:
    explicit JsonPath(std::string_view path)
    {
        std::stringstream ss(path.data());
        std::string token;
        while (std::getline(ss, token, '/'))
        {
            if (!token.empty())
            {
                mTokens.emplace_back(std::move(token));
            }
        }
    }

    bool AreSiblings(const JsonPath& other) const
    {
        if (mTokens.size() == other.mTokens.size() && mTokens.size() >= 1)
        {
            for (size_t i = 0; i < mTokens.size() - 1; ++i)
            {
                if (mTokens[i] != other.mTokens[i])
                {
                    return false;
                }
            }
        
            return mTokens.back() != other.mTokens.back();
        }
		
        return false;
    }

    const std::vector<std::string>& Tokens() const
    {
        return mTokens;
    }

    JsonPath Parent() const
    {
        if (mTokens.empty())
        {
            return JsonPath("");
        }
        return JsonPath(GeneratePathFromTokens(mTokens.begin(), mTokens.end() - 1));
    }    

    bool operator==(const JsonPath& other) const
    {
        return mTokens == other.mTokens;
    }

    bool operator!=(const JsonPath& other) const
    {
        return !(*this == other);
    }

    friend std::ostream& operator<<(std::ostream& os, const JsonPath& path)
    {
        os << GeneratePathFromTokens(path.mTokens.begin(), path.mTokens.end());
        return os;
    }

    struct Hash
    {
        size_t operator()(const JsonPath& path) const
        {
            std::hash<std::string> stringHasher;
            size_t hash = 0;
            for (const auto& token : path.mTokens)
            {
                hash ^= stringHasher(token) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };

private:
    template <typename Iterator>
    static std::string GeneratePathFromTokens(Iterator begin, Iterator end)
    {
        std::ostringstream oss;
        for (Iterator it = begin; it != end; ++it)
        {
            if (it != begin)
            {
                oss << "/";
            }
            oss << *it;
        }
        return "/" + oss.str();
    }

    std::vector<std::string> mTokens;
};

// Specialization of std::hash for JsonPath
namespace std
{
    template <>
    struct hash<JsonPath>
    {
        size_t operator()(const JsonPath& path) const
        {
            return JsonPath::Hash{}(path);
        }
    };
}
