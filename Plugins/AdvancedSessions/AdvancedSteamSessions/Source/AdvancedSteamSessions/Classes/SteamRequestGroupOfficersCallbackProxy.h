// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"

// This is taken directly from UE4 - OnlineSubsystemSteamPrivatePCH.h as a fix for the array_count macro

// @todo Steam: Steam headers trigger secure-C-runtime warnings in Visual C++. Rather than mess with _CRT_SECURE_NO_WARNINGS, we'll just
//	disable the warnings locally. Remove when this is fixed in the SDK
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX

#pragma push_macro("ARRAY_COUNT")
#undef ARRAY_COUNT

#include <steam/steam_api.h>

#pragma pop_macro("ARRAY_COUNT")

#endif

// @todo Steam: See above
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "SteamRequestGroupOfficersCallbackProxy.generated.h"

USTRUCT(BlueprintType, Category = "Online|SteamAPI|SteamGroups")
struct FBPSteamGroupOfficer
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
		FBPUniqueNetId OfficerUniqueNetID; // Uint64 representation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Online|SteamAPI|SteamGroups")
	bool bIsOwner;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintGroupOfficerDetailsDelegate, const TArray<FBPSteamGroupOfficer> &, OfficerList);

UCLASS(MinimalAPI)
class USteamRequestGroupOfficersCallbackProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	// Called when there is a successful results return
	UPROPERTY(BlueprintAssignable)
	FBlueprintGroupOfficerDetailsDelegate OnSuccess;

	// Called when there is an unsuccessful results return
	UPROPERTY(BlueprintAssignable)
	FBlueprintGroupOfficerDetailsDelegate OnFailure;

	// Retruns a list of steam group officers
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online|SteamAPI|SteamGroups")
	static USteamRequestGroupOfficersCallbackProxy* GetSteamGroupOfficerList(UObject* WorldContextObject, FBPUniqueNetId GroupUniqueNetID);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate() override;
	// End of UOnlineBlueprintCallProxyBase interface

private:
	
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	void OnRequestGroupOfficerDetails( ClanOfficerListResponse_t *pResult, bool bIOFailure);
	CCallResult<USteamRequestGroupOfficersCallbackProxy, ClanOfficerListResponse_t> m_callResultGroupOfficerRequestDetails;
#endif

private:

	FBPUniqueNetId GroupUniqueID;
	UObject* WorldContextObject;
};