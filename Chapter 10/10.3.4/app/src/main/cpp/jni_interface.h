
#pragma once

#include <string>
#include <vector>

namespace JNI
{
    void* GetJavaEnvironment();

    void* GetContext();

    bool CheckPermission(const std::string& _rPermission);

    void AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions);
} // namespace JNI
