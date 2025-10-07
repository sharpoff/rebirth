#include <rebirth/core/cvar_system.h>

#include <rebirth/util/logger.h>

CVarSystem *CVarSystem::instance()
{
    static CVarSystem cvarSystem;
    return &cvarSystem;
}

CVar *CVarSystem::getCVar(eastl::string name)
{
    if (cvars.find(name) != cvars.end())
        return &cvars[name];

    logger::logWarn("Failed to get CVar with name - ", name.c_str());
    return nullptr;
}

int *CVarSystem::getCVarInt(eastl::string name)
{
    CVar *cvar = getCVar(name);
    if (cvar && cvar->arrayIndex >= 0) {
        return &intArray[cvar->arrayIndex];
    }

    return nullptr;
}

float *CVarSystem::getCVarFloat(eastl::string name)
{
    CVar *cvar = getCVar(name);
    if (cvar && cvar->arrayIndex >= 0) {
        return &floatArray[cvar->arrayIndex];
    }

    return nullptr;
}

eastl::string *CVarSystem::getCVarString(eastl::string name)
{
    CVar *cvar = getCVar(name);
    if (cvar && cvar->arrayIndex >= 0) {
        return &stringArray[cvar->arrayIndex];
    }

    return nullptr;
}

void CVarSystem::setCVarInt(eastl::string name, int value, eastl::string description)
{
    if (cvars.find(name) != cvars.end()) {
        CVar &cvar = cvars[name];
        if (cvar.arrayIndex >= 0) {
            cvar.description = cvar.description.empty() ? description : cvar.description;
            intArray[cvar.arrayIndex] = value;
        }
    }

    CVar cvar;
    cvar.name = name;
    cvar.description = description;
    cvar.type = CVarType::Int;
    cvar.arrayIndex = intArray.size();

    intArray.push_back(value);
    cvars[name] = cvar;
}

void CVarSystem::setCVarFloat(eastl::string name, float value, eastl::string description)
{
    if (cvars.find(name) != cvars.end()) {
        CVar &cvar = cvars[name];
        if (cvar.arrayIndex >= 0) {
            cvar.description = cvar.description.empty() ? description : cvar.description;
            floatArray[cvar.arrayIndex] = value;
        }
    }

    CVar cvar;
    cvar.name = name;
    cvar.description = description;
    cvar.type = CVarType::Float;
    cvar.arrayIndex = intArray.size();

    floatArray.push_back(value);
    cvars[name] = cvar;
}

void CVarSystem::setCVarString(eastl::string name, eastl::string value, eastl::string description)
{
    if (cvars.find(name) != cvars.end()) {
        CVar &cvar = cvars[name];
        if (cvar.arrayIndex >= 0) {
            cvar.description = cvar.description.empty() ? description : cvar.description;
            stringArray[cvar.arrayIndex] = value;
        }
    }

    CVar cvar;
    cvar.name = name;
    cvar.description = description;
    cvar.type = CVarType::String;
    cvar.arrayIndex = intArray.size();

    stringArray.push_back(value);
    cvars[name] = cvar;
}