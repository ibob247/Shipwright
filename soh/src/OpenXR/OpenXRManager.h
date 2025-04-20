#pragma once
#include <openxr/openxr.h>

class OpenXRManager {
public:
    bool Initialize();
    void Shutdown();

    bool BeginFrame();
    void EndFrame();

private:
    XrInstance instance{XR_NULL_HANDLE};
    XrSession session{XR_NULL_HANDLE};
    XrSystemId systemId{XR_NULL_SYSTEM_ID};

    bool CreateInstance();
    bool CreateSession();
};
