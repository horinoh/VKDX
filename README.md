# VKDX

## VK、DX 共通

### コンパイル
* "warning C4005: '_malloca': macro redefinition" は framework.h 内 windows.h 前に _CRTDBG_MAP_ALLOC を定義すると出なくなる
~~~
#define _CRTDBG_MAP_ALLOC
#include <windows.h>
~~~

### Visual Assist X
* 検索対象に拡張子(inl)を追加する
	* Tool - Options - Text Editor - File Extension - Editor で Microsoft Visual C++ を選択 - Extension に inl を記述 - Add
	* Visual Assist X をリビルドする

### DDS ツール
* DirectXTex https://github.com/Microsoft/DirectXTex
  * DirectXTex\DirectXTex_Desktop_2015.sln を開いて Release、x64 にしてビルドすると実行ファイルが作成される
~~~
DDSView\Bin\Desktop_2015\x64\Release\DDSView.exe
Texassemble\Bin\Desktop_2015\x64\Release\texassemble.exe
Texconv\Bin\Desktop_2015\x64\Release\texconv.exe
Texdiag\Bin\Desktop_2015\x64\Release\texdiag.exe
~~~

* 中間リソースを使用するもの(例えばDDSファイル)は PreBuildEvent で Intermediate から ProjectDir, TargetDir へコピーしている (VS 起動時と exe 直起動時用)
~~~
xcopy /y $(SolutionDir)\Intermediate\Image\UV.dds $(ProjectDir)
xcopy /y $(SolutionDir)\Intermediate\Image\UV.dds $(TargetDir)
~~~

### エディット & コンティニュー
- Tools - Option - Debugging - General - Enable Edit and Continue

### GLTF
 * C++ローダー https://github.com/jessey-git/fx-gltf を同じ階層にクローンする
 * JSON を必要とするので https://github.com/nlohmann/json を同じ階層にクローンする
 * **..\\..\fx-gltf\include** にインクルードパスを通す
~~~
#include <fx/gltf.h>
~~~
 * サンプルデータ https://github.com/KhronosGroup/glTF-Sample-Models を同じ階層にクローンする

### プリコンパイルヘッダーを使う
 * (ここでは)framework.cpp を追加 - 右クリック - Properties - C/C++ - Precompiled Headers - Create(/Yc)
 * Properties - C/C++ - Advanced - Forced Include File に framework.h を指定しているので、framework.cpp に #include "framework.h" は記述しなくて良い

## VK

#### SDK
* https://vulkan.lunarg.com/signin
* インストールすると環境変数 **VK_SDK_PATH**、**VULKAN_SDK** が自動的に作成される
	* VAX : 新しいバージョンをインストールしたらパスが変わるので VAssistX - Visual Assist Options - Performance - Rebuild しておく
	* UE4 : 環境変数 **VULKAN_SDK** は UE4 のコンパイルが通らなくなるので消した `setx VULKAN_SDK ""`
* C/C++ - Preprocessor - Preprocessor Definitions に **VK_USE_PLATFORM_WIN32_KHR** を定義した
* 環境変数 **VK_INSTANCE_LAYERS** を作成しておくか、インスタンス作成持にプログラム中から指定してもよい。
~~~
setx VK_INSTANCE_LAYERS VK_LAYER_LUNARG_standard_validation
~~~
* DLL
	* C/C++ - Preprocessor - Preprocessor Definitions に **VK_NO_PROTOYYPES** を定義しておく
	* %VK_SDK_PATH%\RunTimeInstaller\VulkanRT-XXX-Installer.exe を実行すると DLL がインストールされる
		* SDKのインストール時に自動的に行われている？

* バリデーションレイヤ
	* アプリ と VulkanAPI の間のレイヤ
	* コード中からやらない場合は以下のようにする
		* %VK_SDK_PATH%\Config\vk_layer_settings.txt をデバッグしたい exe と同じ場所へコピーし、環境変数 VK_INSTANCE_LAYER を定義しておく
~~~
xcopy /y %VK_SDK_PATH%\Config\vk_layer_settings.txt $(ProjectDir)
xcopy /y %VK_SDK_PATH%\Config\vk_layer_settings.txt $(TargetDir)
~~~
~~~
setx VK_INSTANCE_LAYERS VK_LAYER_LUNARG_standard_validation
~~~

#### ドライバ
* https://www.khronos.org/vulkan/
	* NVIDIA のドライバアップデート後に vkCreateInstance() でコケるようになったら、恐らく Vulkan ドライバを再インストールすると治る

#### GLM
* https://github.com/g-truc/glm
* 同じ階層に GLM をクローンして **..\\..\glm** にインクルードパスを通した

#### GLI
* https://github.com/g-truc/gli
* 同じ階層に GLI をクローンして **..\\..\gli** にインクルードパスを通した

#### Vulkan-Hpp
* ~~https://github.com/KhronosGroup/Vulkan-Hpp~~ 今は通常インストールに含まれるみたい
	* ここでは未使用

#### シェーダコンパイル
* glslangValidator.exe でコンパイルする、SDKインストールで環境変数 **Path** が通るらしくそのまま使用できる
* Custom Build Tool に以下のように指定した (.exe 直起動もできるように TargetDir にもコピーしている)
	* Outputs : `%(Identity).spv`
	* Description : `GLSL Compiler`
	* Link Objects : `No`
	* Treat Output As Content : `Yes`
	* Command Line :
* Debug
~~~
glslangValidator -H %(Identity) -o %(Identity).spv > %(Identity).asm
xcopy /y %(Identity).spv $(TargetDir) //!< TargetDir にもコピー
~~~
* Release
~~~
glslangValidator -V %(Identity) -o %(Identity).spv
spirv-remap --map all --input %(Identity).spv --output .
xcopy /y %(Identity).spv $(TargetDir)
~~~
* その他オプション
  * -H SPIR-Vコードを標準出力へ
  * -e エントリポイント
  * -S シェーダステージ (vert, frag,...)
  * -D HLSLファイルを指定する場合
  * -D マクロを指定する場合(-Dの後にスペースを入れずに指定)
    
* プロパティシートへの変更は **sln を立ち上げ直さないと反映されない**

#### オンラインコンパイル
* VulkanSDK/バージョン/glslang/CMakeList.txt を使って cmake でプロジェクトを生成
* glslang.sln を開いて glslang, glslangValidator, OGLCompipler, OSDependent, SPIRV, spirv-remap をビルド
	* 64bitで使う場合は x64 を追加してビルドしないとだめみたい
* SPIRV.lib, glslang.lib, OGLCompiler.lib, OSDependent.lib
	* ここでは未使用

#### Visual Studio で GLSL シンタックスハイライトさせる場合
* ~~ShaderHighlights\XXX_vs2015.reg を実行して Visual Studio を再起動~~
* ~~ShaderHighlights\XXX_vs2017.reg を実行して Visual Studio を再起動~~
* GLSL language integration拡張をインストールする

#### デバッグ
* RenderDoc https://renderdoc.org/builds
* RenderDoc 無しで、Debug 版をビルドするには、USE_RENDERDOC の定義をやめる
* 参考 http://www.saschawillems.de/?page_id=2017
	* RenderDocをインストールして起動 - Warning をクリックするとWindowsのレジストリが作られる(初回のみ)
	* Executable Path に exe を指定して、 Launch ボタンでアプリを実行、PrintScreen でシーンをキャプチャしてアプリを閉じる
		* 必要に応じて上記のexe指定を cap ファイルに保存しておく
	* **RenderDoc から実行した場合にしか VK_EXT_DEBUG_MARKER_EXTENSION_NAME は有効にならないみたいなので注意**

## DX

### Visual Studio
 * Visual Studio のインストール時に Universal Windows App Development Tools - Tools and Windows 10 SDK 10.XXX をチェックしておく必要がある
 * インストール済みの場合は「プログラムと機能」から更新インストールする 

#### WinRt
* Microsoft::WRL::ComPtr -> winrt::com_ptr への移行 (参考 https://docs.microsoft.com/ja-jp/windows/uwp/cpp-and-winrt-apis/move-to-winrt-from-wrl)
	* プロジェクト右クリック - Property - All Configurations にする - C/C++ - Language - C++ Language Standard - ISO C++17 Standard を選択
		* デフォルトでは C++14になっているみたい
	* 以下のようなコード変更を行う
~~~
//#include <wrl.h>
#include <winrt/base.h>

//Microsoft::WRL::ComPtr<ID3D12Device> Device;
winrt::com_ptr<ID3D12Device> Dev;

//IID_PPV_ARGS(&XXX)
//IID_PPV_ARGS(XXX.GetAddressOf())
__uuidof(XXX), XXX.put_void()

//IID_PPV_ARGS(XXX.ReleaseAndGetAddressOf())
XXX = nullptr;
__uuidof(XXX), XXX.put_void()

//XXX.Get()
XXX.get()

//&XXX
//XXX.GetAddressOf()
XXX.put()
XXX.put_void()

//XXX.ReleaseAndGetAddressOf()
XXX = nullptr;
XXX.put()
XXX.put_void()

//XXX.Reset()
XXX = nullptr

//XXX.As(&YYY)
XXX->QueryInterface(__uuidof(YYY), YYY.put_void());
winrt::copy_to_abi(XXX, *YYY.put_void());
~~~

### DirectXTK (DDS読み込みに使用)
* https://github.com/Microsoft/DirectXTK12
* DirectXTK_Desktop_2017_Win10.sln を開いてビルド(Debug, Release)
	* D3D12_DESCRIPTOR_RANGE1 がないと言われて、コンパイルが通らない場合は  Windows 10 Anniversary Update SDK が必要(VisualStudioを更新する)
* 同じ階層に DirectXTK12 をクローンして **..\..\DirectXTK12** にパスを通した
* 備考
	* DirectXTex(https://github.com/Microsoft/DirectXTex/wiki/DirectXTex) もある、こちらでもよい

#### シェーダコンパイル
* シェーダは Visual Studio に追加すると自動的にコンパイルされる
* Properties - HLSLCompiler - General - Shader Type を適切に設定しておかないと、頂点シェーダでコンパイルされるので注意
* HLSL Compiler - Output Files - $(OutDir)%(Filename).cso を $(ProjectDir)%(Filename).cso へ変更した
* .exe 直起動もできるように TargetDir にもコピーしている
	* Visual Studio で BuildEvent - Post-Build Event に以下のように指定した
~~~
for %%1 in (*.cso) do xcopy /y %%1 $(TargetDir) //!< TargetDir にもコピー
~~~

#### デバッグ
* WinPixEventRuntimeのインストール
	* ソリューション右クリック - ソリューションのNuGetパッケージの管理 - 参照タブ - WinPixEventRuntimeで検索 - プロジェクトを選択してインストール
		* これをすると #include <pix3.h> が可能になる

 * 参考 https://msdn.microsoft.com/ja-jp/library/hh873204.aspx
	* Alt + F5 で開始 (Debug - Graphics - Start Graphics Debugging)
	* PrintScreen でキャプチャ (Debug - Graphics - Capture Frame)
	* キャプチャしたフレームがサムネイルされる、ダブルクリックすると Analyzer が起動する
		* 下の方に出るので Frame time, Frames per second を閉じないと見えないかも

#### ルートシグネチャをHLSLで指定する
* 参考
	* https://docs.microsoft.com/en-us/windows/desktop/direct3d12/specifying-root-signatures-in-hlsl
* 以下のような define定義を書いたファイル RS.hlsl を準備する
~~~
#define RS "..."
~~~
* コンパイル 
	* fxc.exe は C:\Program Files (x86)\Windows Kits\10\bin\10.0.17763.0\x64 以下などにあるはず
	* fxc の代わりに dxc だとうまくいかなかった (コンパイルはできるが、読み込むと死ぬ)
	* RS.fxo をBlobとして読み込んで使う
	* ファイル名が RS.hlsl、define定義名が RS、出力ファイルが RS.cso の場合の例
~~~
fxc /T rootsig_1_1 RS.hlsl /E RS /Fo RS.cso
~~~
* Visual Studio への追加
	* Proerties - Build Events - Pre Build Event - Command Line に以下のように記述
~~~
fxc /T rootsig_1_1 /E RS $(ProjectName).rs.hlsl /Fo $(ProjectName).rs.cso
~~~
<!--
	* Properties - HLSLCompiler - General - Shader Type を Generate Root Signature Object にする (しなくても大丈夫だが一応しておく)
	* Properties - HLSLCompiler - All Options - Entry Point Name に main とあるのを消す (しなくても大丈夫だが一応しておく)
	* Properties - HLSLCompiler - Command Line に以下のように記述
~~~
/T rootsig_1_1 /E RS
~~~
-->

#### トラブルシューティング
* 「このプロジェクトは、このコンピュータ上にないNugetパッケージを参照しています」と出る場合
	* まず VS を最新にアップデートする、それでダメなら以下を試す
	* .vcproj の <Target>...</Target> を消す
	* WinPixEventRuntimeのアンインストール、インストールを行う
* VK.prop を指定してるのに vulkan.h が無いと言われるとき
	* VK.prop を一旦Removeして、再度Addするとうまく行ったりする

<!-- 
## プロジェクトの追加方法 (自分用覚書)
 * ソリューションを右クリック - Add - New Project で Windows Desktop Application
 * プロジェクトを右クリック - Retarget SDK Verson で 10以上にする
 * プロジェクト右クリック - Property - All Configurations にする - C/C++ - Language - C++ Language Standard - ISO C++17 Standard を選択しておく(vs2019のデフォルトはC++14)
 * プロジェクト右クリック - Property - All Configurations にする - C/C++ - General - Warning Level を Level4、Treat Warnings As Errors を Yes にする

#### DX
 * プロパティマネージャで Add Existing Property Sheet... - Props/NOPRECOMP.props, Props/HLSL.props、Props/RS.props. (Props/DXTK.prop, Props/GLFT.prop)
 * Header Files に Win.h、DX.h、DXExt.h、(DXImage.h) を追加 
 * Source Files に Win.cpp、DX.cpp、DXExt.cpp、(DXImage.cpp) を追加
 * framework.h(旧stdafx.h), XxxDX.h、XxxDX.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)
 * Shader Files フォルダを作成し、シェーダを突っ込む
  * シェーダファイルを右クリック - プロパティ - Configuration Propeties - General
   * Excluded From Build を No
   * Content を Yes
  * 右クリック - プロパティ - HLSL Compiler - General - Shader Type でタイプを適切に選択しておく
* WinPixEventRuntimeのインストール
	* 右クリック - Manage NuGet Packages - Browse - WinPixEventRuntimeで検索 - インストール
* ルートシグネチャ用HLSL XxxDX.rs.hlsl を作成する

#### VK
 * プロパティマネージャで Add Existing Property Sheet... - Props/NOPRECOMP.props, Props/VK.props、Props/GLSL(REMAP).props、Props/GLM.prop、(Props/GLI.prop, Props/GLTF.prop)
 * Header Files に Win.h、VK.h、VKExt.h、(VKImage.h) を追加
 * Source Files に Win.cpp、VK.cpp、VKExt.cpp、(VKImage.cpp) を追加
 * framework.h(旧stdafx.h)、XxxVK.h、XxxVK.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)
 * Shader Files フォルダを作成し、シェーダを突っ込む
  * 拡張子を glslangValidator に沿うようにタイプを選択しておく。(.vert、.frag、...)
  * 右クリック - プロパティ - Configuration Propeties - General
    * Excluded From Build を No
    * Content を Yes
    * Item Type を Custom Build Tool
   * 適用 - Custom Build Tool 項目が追加されるので GLSL Compiler になっていることを確認 (↑のプロパティを先に設定しておくこと)
 -->

<!-- 
## GLTF
 * チュートリアル https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/README.md
 * サンプルデータ https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0
 * C++ローダー https://github.com/jessey-git/fx-gltf
 * JSON https://github.com/nlohmann/json
 * DXローダー(使用していない) https://github.com/microsoft/glTF-SDK

## FBX
 * SDKをインストールする
 * 環境変数 **FBX_SDK_PATH** を定義しておく
 * 環境変数 **Path** に DLL のパスを通しておく
 ~~~
 Path=%Path%:%FBX_SDK_PATH%\lib\vs2015\x64\debug
 Path=%Path%:%FBX_SDK_PATH%\lib\vs2015\x64\release
 ~~~
 ~~~
 #include <fbxsdk.h
 #pragma comment(lib, "vs2015\\x64\\debug\\libfbxsdk.lib")
 ~~~
* サンプルデータは $(FBX_SDK_PATH)\samples 以下にある

## DRACO
 * CMakeでプロジェクトを作成
 * 開いて Debug, Release をビルド
 ~~~
 #include "draco/compression/decode.h"
 #pragma comment(lib, "draco.lib")
 #pragma comment(lib, "dracodec.lib")
 #pragma comment(lib, "dracoenc.lib")
 ~~~
* サンプルデータは draco\testdata 以下にある

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

<!--
TODO

# VK
- ウインドウサイズ変更時の処理 OnSize() スワップチェインのリサイズ
- デプスステンシルを有効にして試してない(Billboard)
- コンピュートの検証(テクスチャを準備する)
- ストレージバッファ、ユニフォームテクセルバッファ、ストレージテクセルバッファの検証
- テクスチャ読み込み現状ミップマップ１のみ

# DX
- ウインドウサイズ変更時の処理 OnSize() スワップチェインのリサイズ
- コンピュートの検証(テクスチャを準備する)
- コマンドリスト、グラフィクスコマンドリストまわりをまとめる
- クリアカラーまわりがVKと同様にできるか検証
- テクスチャ読み込み現状ミップマップ１のみ

# 共通
* HDR対応 環境がないので保留
* バッファ毎にデバイスメモリを確保している、大きなデバイスメモリを確保してバッファはその一部を使用するようにしたほうが良い
* ポストプロセス
* プロシージャルテクスチャ
* Gバッファ(MRT)
* シャドウマップ
* サンプラの整理
	DXではシェーダ可視性やレジスタ番号も入っているのでどう扱うか

* テクスチャ
	* ディスプレースメント
	* 圧縮テクスチャ
	* キューブマップ(環境マップ)

* Gバッファ
	* シャドウマップ
	* SSAO
 --> 