# 外部ライブラリ

## [DirectXTK](https://github.com/Microsoft/DirectXTK12)
* DirectXTK_Desktop_20XX_Win10.sln を開いて x64 をビルド (Debug, Release)
	* D3D12_DESCRIPTOR_RANGE1 がないと言われて、コンパイルが通らない場合は  Windows 10 Anniversary Update SDK が必要 (VisualStudioを更新する)

## GLTF
 * NuGetPackage で Microsoft.glTF.CPP をインストール
 	- min, max 関連でコンパイルエラーになる場合、windows.h より前に NOMINMAX を定義する
    	~~~
    	#define NOMINMAX 
    	#include <windows.h>
    	~~~

 	* リンカエラー 4099 が出る (#pragma では回避できない) ので以下のようにしている 
		* Configuration Properties - Linker - CommandLine - AdditionalOptions - /ignore:4099
* [サンプルプログラム](https://github.com/microsoft/glTF-SDK)
 * [サンプルモデル](https://github.com/KhronosGroup/glTF-Sample-Models)
 * [チュートリアル](https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/README)

 <!--
 * https://github.com/syoyo/tinygltf をサブモジュール化した
 * C++ローダー https://github.com/jessey-git/fx-gltf をサブモジュール化した
	* .gitmoudles に以下を追加
		~~~
		ignore = dirty
		~~~
 * JSON を必要とするので https://github.com/nlohmann/json をサブモジュール化した
 * **..\\..\fx-gltf\include** にインクルードパスを通す
~~~
#include <fx/gltf.h>
~~~
 * サンプルデータ 
 	* fx-gltfのサブモジュールに含まれるのを使用する
-->

## [FBX SDK](https://aps.autodesk.com/developer/overview/fbx-sdk)
 * SDK をインストールする
 * ここでは環境変数 **FBX_SDK_PATH** を定義しておく
	* FBX_SDK_PATH の例
	~~~
	FBX_SDK_PATH=C:\Program Files\Autodesk\FBX\FBX SDK\20XX.X.X
	~~~
 * 環境変数 **Path** に DLL のパスを通しておく
	* DLL のパスの例
	~~~
	$(FBX_SDK_PATH)\lib\x64\debug
	$(FBX_SDK_PATH)\lib\x64\release
	~~~
 ~~~
 #define FBXSDK_SHARED //!< DLL を使用する場合は定義する
 #include <fbxsdk.h>
 #pragma comment(lib, "libfbxsdk.lib")
 ~~~
* Fbx.h は DXExt.h や VKExt.h 等よりも前に include する
* サンプルデータは $(FBX_SDK_PATH)\samples 以下にある
	* InmportScene が参考になる
	* 普通にインストールすると管理者権限が必要な場所にあるので、どこかにコピーする
	* 以下のようにしてソリューションファイルを生成
	~~~
	$cd ImportScene
	$cmake -G "Visual Studio 16"
	~~~

## [DRACO](https://github.com/google/draco)
 * CMakeでプロジェクトを作成
	* 同じ階層に draco_build フォルダを作成して、出力先として指定する
 * draco.sln を開いて Debug, Release をビルド
 ~~~
 #include "draco_build/compression/decode.h"
 #pragma comment(lib, "draco.lib")
 #pragma comment(lib, "dracodec.lib")
 #pragma comment(lib, "dracoenc.lib")
 ~~~
* サンプルデータは draco\testdata 以下にある
* エンコード、デコード例
	* フォーマットは .ply, .obj が .drc へ変換可能
	~~~
	$draco_encoder -i XXX.ply -o YYY.drc
	$draco_decoder -i YYY.drc -o ZZZ.obj
	~~~

## DirectXMesh
* [DirectXMesh](https://github.com/microsoft/DirectXMesh.git)
	* DirectXMesh_Desktop_20XX_Win10.sln を開いて x64 Debug, Release をビルド
	* meshconver を使う場合
		* DirectXMesh_Desktop_20XX_Win10 ではなくて、**DirectXMesh_Desktop_20XX.sln** を開いてビルドしないと作られないので注意

## [LeapMotion](https://developer.leapmotion.com/sdk-leap-motion-controller/)
 * SDK をダウンロードし解凍、ドライバをインストールしておく
 * LeapSDK を適当な場所へ配置
	* 配置先を環境変数 LEAP_SDK_PATH として定義しておく
	* LeapSDK\lib\x64 を環境変数にPATHに通しておく
	* Leap.h USE_LEAP 定義を有効にする
 * [ドキュメント](https://developer.leapmotion.com/documentation/v4/index.html)

<!--
 ## OPENCV
 * 環境変数 **OPENCV_SDK_PATH** を定義しておく
 * 環境変数 **Path** に DLL のパスを通しておく
 ~~~
 Path=%Path%:%OPENCV_SDK_PATH%\x64\vc14\bin
 ~~~
 ~~~
 #include <opencv2/opencv.hpp>
 #pragma comment(lib, "opencv_core310.lib")
 ...
 ~~~

 ## SDL
 * 環境変数 **SDL_SDK_PATH** を定義しておく
 * 環境変数 **Path** に DLL のパスを通しておく
 ~~~
 Path=%Path%:%SDL_SDK_PATH%\lib\x64
 ~~~
 ~~~
 #include <SDL.h>
 #pragma comment(lib, "SDL2.lib")
 ~~~
 -->
 