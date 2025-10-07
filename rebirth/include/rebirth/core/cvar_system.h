#pragma once

#include "EASTL/unordered_map.h"
#include "EASTL/vector.h"
#include "EASTL/string.h"

enum class CVarType
{
    Int,
    Float,
    String
};

struct CVar
{
    eastl::string name;
    eastl::string description;
    CVarType type;
    size_t arrayIndex;
};

class CVarSystem
{
public:
    static CVarSystem *instance();

    int *getCVarInt(eastl::string name);
    float *getCVarFloat(eastl::string name);
    eastl::string *getCVarString(eastl::string name);

    void setCVarInt(eastl::string name, int value, eastl::string description = "");
    void setCVarFloat(eastl::string name, float value, eastl::string description = "");
    void setCVarString(eastl::string name, eastl::string value, eastl::string description = "");
private:
    CVar *getCVar(eastl::string name);

    CVarSystem() {};
    CVarSystem(CVarSystem const &) = delete;
    void operator=(CVarSystem const &) = delete;

    eastl::unordered_map<eastl::string, CVar> cvars;

    eastl::vector<int> intArray;
    eastl::vector<float> floatArray;
    eastl::vector<eastl::string> stringArray;
};