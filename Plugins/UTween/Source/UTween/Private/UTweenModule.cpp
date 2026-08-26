#include "UTweenModule.h"

DEFINE_LOG_CATEGORY(LogUTween);

void FUTweenModule::StartupModule()
{
    UE_LOG(LogUTween, Log, TEXT("UTween module started"));
}

void FUTweenModule::ShutdownModule()
{
    UE_LOG(LogUTween, Log, TEXT("UTween module shut down"));
}

IMPLEMENT_MODULE(FUTweenModule, UTween)
