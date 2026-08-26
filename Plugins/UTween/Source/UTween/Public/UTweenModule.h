#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

UTWEEN_API DECLARE_LOG_CATEGORY_EXTERN(LogUTween, Log, All);

class FUTweenModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
