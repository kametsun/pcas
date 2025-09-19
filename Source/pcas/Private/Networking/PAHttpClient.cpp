#include "Networking/PAHttpClient.h"

#include "Constants/PAConstants.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Containers/StringConv.h"
#include "Templates/SharedPointer.h"
#include "Misc/Guid.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogPAHttp);

void UPAHttpClient::Initialize(UWorld* InWorld)
{
	CachedWorld = InWorld;
}

void UPAHttpClient::GetJson(const FString& Url, FJsonResponseCallback Callback)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	TSharedRef<FJsonResponseCallback> CallbackRef = MakeShared<FJsonResponseCallback>(MoveTemp(Callback));
	Request->OnProcessRequestComplete().BindLambda([
		this,
		CallbackRef
	](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		HandleJsonResponse(Req, Response, bWasSuccessful, *CallbackRef);
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogPAHttp, Error, TEXT("Failed to start GET request: %s"), *Url);
		(*CallbackRef)(false, nullptr, TEXT("Failed to start request"));
	}
}

void UPAHttpClient::PostMultipart(const FString& Url,
	const TArray<uint8>& FileData,
	const FString& FileFieldName,
	const FString& FileName,
	const FString& FileContentType,
	const FString& JsonFieldName,
	const FString& JsonPayload,
	FRawResponseCallback Callback,
	int32 Attempt)
{
	TSharedRef<FRawResponseCallback> CallbackRef = MakeShared<FRawResponseCallback>(MoveTemp(Callback));
	PostMultipartInternal(Url, FileData, FileFieldName, FileName, FileContentType, JsonFieldName, JsonPayload, CallbackRef, Attempt);
}

void UPAHttpClient::PostMultipartInternal(const FString& Url,
	const TArray<uint8>& FileData,
	const FString& FileFieldName,
	const FString& FileName,
	const FString& FileContentType,
	const FString& JsonFieldName,
	const FString& JsonPayload,
	TSharedRef<FRawResponseCallback> CallbackRef,
	int32 Attempt)
{
	FString Boundary = FString::Printf(TEXT("----PointCloudAutoBoundary%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
	const FString LineEnd = TEXT("\r\n");

	TArray<uint8> Payload;
	Payload.Reserve(FileData.Num() + JsonPayload.Len() + 512);

	auto AppendString = [&Payload](const FString& Str)
	{
		FTCHARToUTF8 Converter(*Str);
		Payload.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
	};

	AppendString(FString::Printf(TEXT("--%s%s"), *Boundary, *LineEnd));
	AppendString(FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"%s"), *FileFieldName, *FileName, *LineEnd));
	AppendString(FString::Printf(TEXT("Content-Type: %s%s%s"), *FileContentType, *LineEnd, *LineEnd));
	Payload.Append(FileData);
	AppendString(LineEnd);

	AppendString(FString::Printf(TEXT("--%s%s"), *Boundary, *LineEnd));
	AppendString(FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"%s"), *JsonFieldName, *LineEnd));
	AppendString(FString::Printf(TEXT("Content-Type: application/json%s%s"), *LineEnd, *LineEnd));
	AppendString(JsonPayload);
	AppendString(LineEnd);

	AppendString(FString::Printf(TEXT("--%s--%s"), *Boundary, *LineEnd));

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContent(Payload);

	Request->OnProcessRequestComplete().BindLambda([
		this,
		CallbackRef,
		Url,
		FileData,
		FileFieldName,
		FileName,
		FileContentType,
		JsonFieldName,
		JsonPayload,
		Attempt
	](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			(*CallbackRef)(true, TEXT(""));
			return;
		}

		UE_LOG(LogPAHttp, Warning, TEXT("Upload failed (attempt %d): %s"), Attempt + 1, Response.IsValid() ? *Response->GetContentAsString() : TEXT("no response"));

		const int32 MaxAttempts = UPAConstants::GetUploadRetryCount();
		if (Attempt + 1 < MaxAttempts)
		{
			double Delay = UPAConstants::GetUploadRetryBackoffSeconds() * FMath::Pow(2.0, Attempt);
			RetryDelayed([this, Url, FileData, FileFieldName, FileName, FileContentType, JsonFieldName, JsonPayload, CallbackRef, Attempt]()
			{
				PostMultipartInternal(Url, FileData, FileFieldName, FileName, FileContentType, JsonFieldName, JsonPayload, CallbackRef, Attempt + 1);
			}, Delay);
			return;
		}

		const FString ErrorText = Response.IsValid() ? FString::Printf(TEXT("Upload failed with code %d"), Response->GetResponseCode()) : TEXT("Upload request failed");
		(*CallbackRef)(false, ErrorText);
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogPAHttp, Error, TEXT("Failed to start POST request: %s"), *Url);
		(*CallbackRef)(false, TEXT("Failed to start upload"));
	}
}

void UPAHttpClient::HandleJsonResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FJsonResponseCallback& Callback)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		const FString Message = TEXT("HTTP request failed");
		UE_LOG(LogPAHttp, Error, TEXT("%s"), *Message);
		Callback(false, nullptr, Message);
		return;
	}

	const int32 Code = Response->GetResponseCode();
	if (!EHttpResponseCodes::IsOk(Code))
	{
		const FString Message = FString::Printf(TEXT("Unexpected status code: %d"), Code);
		UE_LOG(LogPAHttp, Error, TEXT("%s"), *Message);
		Callback(false, nullptr, Message);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const FString Body = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		const FString Message = TEXT("Failed to parse JSON");
		UE_LOG(LogPAHttp, Error, TEXT("%s"), *Message);
		Callback(false, nullptr, Message);
		return;
	}

	Callback(true, JsonObject, TEXT(""));
}

void UPAHttpClient::RetryDelayed(const TFunction<void()>& Action, double DelaySeconds)
{
	if (!CachedWorld.IsValid())
	{
		Action();
		return;
	}

	FTimerDelegate Delegate = FTimerDelegate::CreateLambda([Action]()
	{
		Action();
	});

	FTimerHandle Handle;
	CachedWorld->GetTimerManager().SetTimer(Handle, Delegate, DelaySeconds, false);
}
