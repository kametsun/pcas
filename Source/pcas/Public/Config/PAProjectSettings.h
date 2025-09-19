#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PAProjectSettings.generated.h"

/**
 * プロジェクト全体で共有する設定値。
 * DefaultGame.ini などから上書き可能。
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Point Cloud Auto Shooting"))
class UPCAProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPCAProjectSettings();

	// Backend
	UPROPERTY(EditDefaultsOnly, Config, Category="Backend", meta=(ToolTip="バックエンドのオリジン URL"))
	FString BackendOrigin;

	UPROPERTY(EditDefaultsOnly, Config, Category="Backend", meta=(ToolTip="3D Tiles リストのパス"))
	FString TilesListPath;

	UPROPERTY(EditDefaultsOnly, Config, Category="Backend", meta=(ToolTip="巡回ルート API のパス"))
	FString RoutesPath;

	UPROPERTY(EditDefaultsOnly, Config, Category="Backend", meta=(ToolTip="画像アップロード API のパス"))
	FString UploadPath;

	// Cesium origin
	UPROPERTY(EditDefaultsOnly, Config, Category="Cesium", meta=(ToolTip="CesiumGeoreference の原点（緯度）"))
	double OriginLatitude;

	UPROPERTY(EditDefaultsOnly, Config, Category="Cesium", meta=(ToolTip="CesiumGeoreference の原点（経度）"))
	double OriginLongitude;

	UPROPERTY(EditDefaultsOnly, Config, Category="Cesium", meta=(ToolTip="CesiumGeoreference の原点（高さ, m）"))
	double OriginHeight;

	UPROPERTY(EditDefaultsOnly, Config, Category="Cesium", meta=(ToolTip="位置情報APIからの高度に加算する補正 (m)"))
	double AltitudeOffsetMeters;

	// Capture config
	UPROPERTY(EditDefaultsOnly, Config, Category="Capture", meta=(ToolTip="スクリーンショットの保存先 (Saved からの相対パス)"))
	FString ScreenshotSubdirectory;

	UPROPERTY(EditDefaultsOnly, Config, Category="Capture", meta=(ToolTip="スクリーンショット解像度"))
	FIntPoint CaptureResolution;

	UPROPERTY(EditDefaultsOnly, Config, Category="Capture", meta=(ToolTip="SceneCaptureで手動露出を使用する"))
	bool bCaptureUseManualExposure;

	UPROPERTY(EditDefaultsOnly, Config, Category="Capture", meta=(ToolTip="手動露出使用時の露出バイアス"))
	float CaptureManualExposureBias;
	// Timing
	UPROPERTY(EditDefaultsOnly, Config, Category="Timing", meta=(ToolTip="Tileset 初期化を待つ秒数"))
	double TilesLoadGracePeriodSeconds;

	UPROPERTY(EditDefaultsOnly, Config, Category="Timing", meta=(ToolTip="撮影前に静止する秒数"))
	double PreCaptureStabilizationSeconds;

	// Upload
	UPROPERTY(EditDefaultsOnly, Config, Category="Upload", meta=(ToolTip="アップロード失敗時の最大リトライ回数"))
	int32 UploadRetryCount;

	UPROPERTY(EditDefaultsOnly, Config, Category="Upload", meta=(ToolTip="アップロードリトライの初期待機秒数"))
	double UploadRetryBackoffSeconds;

	// Orientation
	UPROPERTY(EditDefaultsOnly, Config, Category="Orientation", meta=(ToolTip="方位→Yaw 変換時の補正角 (度)"))
	double DirectionOffsetDegrees;

	// Lifecycle
	UPROPERTY(EditDefaultsOnly, Config, Category="Lifecycle", meta=(ToolTip="全ルート完了後にアプリを終了する"))
	bool bExitApplicationOnFinish;

	UPROPERTY(EditDefaultsOnly, Config, Category="Lifecycle", meta=(ToolTip="エディタ実行時は終了せずログのみとする"))
	bool bExitEditorOnFinish;

	// Map
	UPROPERTY(EditDefaultsOnly, Config, Category="Maps", meta=(ToolTip="起動時にロードするワールド名"))
	FString DefaultWorldMapName;

	// Movement
	UPROPERTY(EditDefaultsOnly, Config, Category="Movement", meta=(ToolTip="カメラ移動の補間秒数"))
	double MovementBlendTimeSeconds;

	UPROPERTY(EditDefaultsOnly, Config, Category="Movement", meta=(ToolTip="方向転換の補間秒数"))
	double RotationBlendTimeSeconds;

	virtual FName GetCategoryName() const override { return TEXT("Project"); }
};
