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

### 素材
* 以下の素材を使わせてもらっている (Thanks to these asetts)
	* [cc0textures](https://cc0textures.com/)
	* [polyhaven](https://polyhaven.com/)
	* [The Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/)

### テクスチャ
#### DDS
* [DirectXTex](https://github.com/Microsoft/DirectXTex)
  * DirectXTex\DirectXTex_Desktop_2019.sln を開いて Release、x64 にして必要なもの(DDDSView, Texassemble, Texconv, Texdiag)をビルドすると実行ファイルが作成される
	~~~
	DDSView\Bin\Desktop_2019\x64\Release\DDSView.exe
	Texassemble\Bin\Desktop_2019\x64\Release\texassemble.exe
	Texconv\Bin\Desktop_2019\x64\Release\texconv.exe
	Texdiag\Bin\Desktop_2019\x64\Release\texdiag.exe
	~~~
* [DDSサムネイル表示](https://sourceforge.net/projects/sagethumbs/)
* 非 dds テクスチャは DirectX Texture Tool で開いた後、Save As で dds として保存できる
* [cmftStudio](https://github.com/dariomanesku/cmftStudio) 
	* HDR テクスチャを DDSキューブマップにするのに使用 (Used to convert HDR to DDS cubemap)
		* アプリを起動
		* 右側から Skybox をクリック - Browse... - HDR テクスチャを選択して - Load
		* 必要に応じて Tonemap を調整
		* Save - DDS - Cubemap - BGRA8 - Save でファイルを出力
#### KTX
* [KTX](https://www.khronos.org/ktx/)
* KTX-Software-X.X.X-win64.exe をインストール
	* [ドキュメント](https://github.khronos.org/KTX-Software/ktxtools.html)
		* ktx2check ... 有効な ktx かどうかのチェック
		* ktx2ktx2 ... ktx → ktx2 への変換
		* ktxinfo ... 情報の表示
		* ktxsc ... 圧縮
		* toktx ... ktx への変換 (.jpg や .png から変換できる)
			~~~
			REM .jpg -> .ktx 変換の例
			$for %i in (*.jpg) do toktx %~ni.ktx %~ni.jpg
			~~~
* [ビューア](https://github.com/kopaka1822/ImageViewer)

### 外部ライブラリ
#### GLTF
 * https://github.com/syoyo/tinygltf をサブモジュール化した
 * サンプルは https://github.com/KhronosGroup/glTF-Sample-Models
 <!--
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

#### FBX
 * SDKをインストールする
 * ここでは環境変数 **FBX_SDK_PATH** を定義しておく
	* FBX_SDK_PATH の例
	~~~
	FBX_SDK_PATH=C:\Program Files\Autodesk\FBX\FBX SDK\2020.2.1
	~~~
 * 環境変数 **Path** に DLL のパスを通しておく
	* DLL のパスの例
	~~~
	$(FBX_SDK_PATH)\lib\vs2019\x64\debug
	$(FBX_SDK_PATH)\lib\vs2019\x64\release
	~~~
 ~~~
 #define FBXSDK_SHARED //!< DLL を使用する場合は定義する
 #include <fbxsdk.h>
 #ifdef _DEBUG
 #pragma comment(lib, "vs2019\\x64\\debug\\libfbxsdk.lib")
 #else
 #pragma comment(lib, "vs2019\\x64\\release\\libfbxsdk.lib")
 #endif
 ~~~
* Fbx.h は DXExt.h や VKExt.h 等よりも前に include する
* サンプルデータは $(FBX_SDK_PATH)\samples 以下にある
	* InmportScene が参考になる
	* 普通にインストールすると管理者権限が必要な場所にあるので、どこかにコピーする
	* 以下のようにしてソリューションファイルを生成
	~~~
	$cd ImportScene
	//$cmake -G "Visual Studio 15 Win64"
	$cmake -G "Visual Studio 16"
	~~~

#### DRACO
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
"""
$draco_encoder -i XXX.ply -o YYY.drc
$draco_decoder -i YYY.drc -o ZZZ.obj
"""

#### DirectXMesh
* [DirectXMesh](https://github.com/microsoft/DirectXMesh.git)
	* DirectXMesh_Desktop_2019_Win10.sln を開いて x64 Debug, Release をビルド
	* meshconver を使う場合
		* DirectXMesh_Desktop_2019_Win10 ではなくて、**DirectXMesh_Desktop_2019.sln** を開いてビルドしないと作られないので注意

#### LeapMotion
 * [SDK](https://developer.leapmotion.com/sdk-leap-motion-controller/) をダウンロードし解凍、ドライバをインストールしておく
 * LeapSDKを適当な場所へ配置
	* 配置先を環境変数 LEAP_SDK_PATH として定義しておく
	* LeapSDK\lib\x64 を環境変数にPATHに通しておく
	* Leap.h USE_LEAP 定義を有効にする
 * [ドキュメント](https://developer.leapmotion.com/documentation/v4/index.html)

#### HoloPlay
 * [HoloPlayService](https://lookingglassfactory.com/software) をインストールしておく
	* HoloPlayStudio も必要に応じてインストールしておく
 * HoloPlayCoreSDK\HoloPlayCore\dylib\Win64 を環境変数にPATHに通しておく
 * Holo.h USE_HOLO 定義を有効にする
 * [SDK](https://github.com/Looking-Glass/HoloPlayCoreSDK) はサブモジュールとしてある
 * USB-C, HDMI を PC に接続
 * スタート - 設定 - システム - ディスプレイ - マルチディスプレイ - 画面表示を拡張する - Looking Glass の画面を選択 - 拡大縮小とレイアウトを 100% にする

### プリコンパイルヘッダーを使う
 * (ここでは)framework.cpp を追加 - 右クリック - Properties - C/C++ - Precompiled Headers - Create(/Yc)
 * Properties - C/C++ - Advanced - Forced Include File に framework.h を指定しているので、framework.cpp に #include "framework.h" は記述しなくて良い

### Gitサブモジュール
 * TortoiseGit - Submodule update - 対象(Path)を選択 - Remote tracking branch にチェックを入れて - OK でサブモジュールを最新のものに更新できる

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
* サブモジュール化した **..\\..\glm** にインクルードパスを通した
	* .gitmoudles に以下を追加
		~~~
		ignore = dirty
		~~~
* マトリクスの乗算順序が DirectXMath では異なるので注意
	- GLSL : 列優先 (Row Major)
	- HLSL : 列優先 (Row Major)
	- glm : 列優先 (Row Major)
		- Projection * View * World
	- DirectXMath : 行優先 (Column Major)
		- World * View * Projection
#### GLI
* https://github.com/g-truc/gli
* サブモジュール化した **..\\..\gli** にインクルードパスを通した
	* .gitmoudles に以下を追加
		~~~
		ignore = dirty
		~~~

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

#### Visual Studio で GLSL, HLSL シンタックスハイライトさせる場合
* Visual Studio - Extensions - Manage Extensions
	* GLSL language integration 拡張を検索してインストールする
	* HLSL Tools for Visual Studio 拡張を検索してインストールする

#### デバッグ
* RenderDoc https://renderdoc.org/builds
	* RenderDoc 無しで、Debug 版をビルドするには、USE_RENDERDOC の定義をやめる
	* 参考 http://www.saschawillems.de/?page_id=2017
		* RenderDocをインストールして起動 - Warning をクリックするとWindowsのレジストリが作られる(初回のみ)
		* Executable Path に exe を指定して、 Launch ボタンでアプリを実行、PrintScreen でシーンをキャプチャしてアプリを閉じる
			* 必要に応じて上記のexe指定を cap ファイルに保存しておく
		* **RenderDoc から実行した場合にしか VK_EXT_DEBUG_MARKER_EXTENSION_NAME は有効にならないみたいなので注意**
* Nsight https://developer.nvidia.com/nsight-graphics
	* .

## DX

### Visual Studio
 * Visual Studio のインストール時に Universal Windows App Development Tools - Tools and Windows 10 SDK 10.XXX をチェックしておく必要がある
 * インストール済みの場合は「プログラムと機能」から更新インストールする 

#### WinRt
* Microsoft::WRL::ComPtr -> winrt::com_ptr への移行 (参考 https://docs.microsoft.com/ja-jp/windows/uwp/cpp-and-winrt-apis/move-to-winrt-from-wrl)
	* プロジェクト右クリック - Property - All Configurations にする - C/C++ - Language - C++ Language Standard - ~~std:c++17~~std:c++latest を選択
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
* DirectXTK_Desktop_2019_Win10.sln を開いてx64をビルド(Debug, Release)
	* D3D12_DESCRIPTOR_RANGE1 がないと言われて、コンパイルが通らない場合は  Windows 10 Anniversary Update SDK が必要(VisualStudioを更新する)
* サブモジュール化したので再帰的にクローンする
	* .gitmoudles に以下を追加
		~~~
		ignore = dirty
		~~~
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
	- 書式例
		~~~
		RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
		DescriptorTable(CBV(b0, space=0), visibility=SHADER_VISIBILITY_GEOMETRY)
		DescriptorTable(SRV(t0, space=0), visibility=SHADER_VISIBILITY_PIXEL)
		DescriptorTable(Sampler(s0, space=0), visibility=SHADER_VISIBILITY_PIXEL)
		StaticSampler(s0,
			space=0,
			filter=FILTER_MIN_MAG_MIP_LINEAR, 
			maxLOD=1.f, 
			maxAnisotropy=0, 
			comparisonFunc=COMPARISON_NEVER,
			visibility=SHADER_VISIBILITY_PIXEL)
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
* D3D12_SHADER_VISIBILITY_ALLについて
	* ハードウェアによってはコストがかかる
	* バインド名を様々なステージで使えなくなる
		- ALLにしなければ、t0:SHADER_VISIBILITY_VERTEX と t0:SHADER_VISIBILITY_PIXEL が使える

<!--
	* Properties - HLSLCompiler - General - Shader Type を Generate Root Signature Object にする (しなくても大丈夫だが一応しておく)
	* Properties - HLSLCompiler - All Options - Entry Point Name に main とあるのを消す (しなくても大丈夫だが一応しておく)
	* Properties - HLSLCompiler - Command Line に以下のように記述
~~~
/T rootsig_1_1 /E RS
~~~
-->

#### 条件コンパイル対応
- USE_VIEWPORT_Y_UP
	- *VK
- USE_INDIRECT
	- FullscreenDX, FullscreenVK
	- MeshShaderDX, MeshShaderVK
- USE_PIPELINE_SERIALIZE
	- ALL
- USE_HLSL_ROOTSIGNATRUE
	- *DX
- USE_BUNDLE, USE_SECONDARY_COMMAND_BUFFER
	- ParametricSurfaceDX, ParametricSurfaceVK
- USE_STATIC_SAMPLER, USE_IMMUTABLE_SAMPLER
	- TextureDX, TextureVK
- USE_MANUAL_CLEAR
	- ClearVK
- USE_PUSH_CONSTANTS, USE_ROOT_CONSTANTS
	- TriangleDX, TriangleVK
- USE_DEPTH
	- ToonDX, ToonVK, RenderTargetDX, RenderTargetVK
- USE_SCREENSPACE_WIREFRAME
	- ToonDX, ToonVK
- USE_DISTANCE_FUNCTION
	- FullscreenDX, FullscreenVK
- USE_SPECIALIZATION_INFO
	- ParametricSurfaceVK
- USE_PUSH_DESCRIPTOR
	- BillboardVK
- USE_PARALLAX_MAP
	- NormalMapDX, NormalMapVK
	- TODO
- USE_SEPARATE_SAMPLER
	- NormalMapVK
- USE_SKY_DOME
	- CubeMapDX, CubeMapVK
- USE_GBUFFER_VISUALIZE
	- DeferredDX, DeferredVK
- USE_SHADOWMAP_VISUALIZE
	- ShadowMapDX, ShadowMapVK
	- TODO
- USE_FULL_SCREEN
	- TODO
- USE_HDR
	- TriangleDX
	- TODO(ハードウェア入手後)
- USE_SHADER_REFLECTION
	- TriangleDX
		- dxc 使用時、dxcompiler.dll が無いと怒られる場合は C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64 とかに存在するので、環境変数 Path に通しておく必要がある
	- *VK 
		- TODO
- USE_SHADER_BLOB_PART
	- TriangleDX
- USE_SUBPASS
	- RenderTargetVk
		- 参考) https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/
		- あるサブパスで書き込まれたフレームバッファアタッチメントに対して、別のサブパスで「同一のピクセル」を読み込むことができる (ただし「周辺のピクセル」は読めないので用途は要考慮)
		- 各サブパスのアタッチメントは１つのフレームバッファにまとめてエントリする
			- VkRenderPassBeginInfoの引数として渡すため、同一パス(サブパス)で完結するにはまとめる必要がある
		- VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENTを指定する
		- vkCmdNextSubpass(でコマンドを次のサブパスへ進める
		- シェーダ内での使用例
			~~~
			layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput XXX;
			vec3 Color = subpassLoad(XXX).rgb;
			~~~
- USE_UPDATE_DESCRIPTOR_SET_WITH_TEMPLATE
	- DisplacementVK
- USE_HOLO
	- HoloDX, HoloVK
- USE_LEAP
	- LeapDX, LeapVK
	
#### レイトレーシング
- シェーダ
	- DX
		- HLSLCompiler - Shader Type には Library を選択する
		~~~
		[shader("raygeneration")]
		void MyRayGen(){}
		[shader("intersection")]
		void MyIntersection()
		[shader("anyhit")]
		void MyAnyHit(inout MYPAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
		[shader("closesthit")]
		void MyClosestHit(inout MYPAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA){}
		[shader("miss")]
		void MyMiss(inout MYPAYLOAD Payload){}
		[shader("callable")]
		void MyCallable(inout MYCALLABLE In)
		~~~
	- VK 
		~~~
		.rgen
		.rint
		.rahit
		.rchit
		.rmiss
		.rcall
		~~~
	- RayGeneration
	- ClosestHit
	- Miss
	
	- Intersection
	- AnyHit
	- Callable
	
#### トラブルシューティング
* 「このプロジェクトは、このコンピュータ上にないNugetパッケージを参照しています」と出る場合
	* まず VS を最新にアップデートする、それでダメなら以下を試す
	* .vcproj の <Target>...</Target> を消す
	* WinPixEventRuntimeのアンインストール、インストールを行う
* *.prop を指定してるのに ～が無いとか言われるとき
	* *.prop を一旦Removeして、再度Addするとうまく行ったりする

<!-- 
## プロジェクトの追加方法 (自分用覚書)
 * Windows Desktop Application を選択
 * Source Files に framework.cpp(内容空で作成)を追加
 	* framewordk.cpp に対してのみ C/C++ - Precompiled headers - Precompiled Header を Create(/Yc) にする
 * framework.h(旧stdafx.h)は既存のものを参考に編集 (#pragma region Code でマークしてある)

#### DX
 * プロパティマネージャで Add Existing Property Sheet... - Props/DX_DEBUG.props, DX_RELEASE.props もしくはテクスチャが必要な場合は Props/DX_DEBUG_TEX.prop, Props/DX_RELEASE_TEX.prop 
	* 必要に応じて Props/GLTF.prop, Props/HOLO.prop, Props/LEAP.prop, Props/FBX.prop, Props/DRACO.prop, Prop/DXMESH.prop 等を追加

 * Header Files に Win.h, DX.h, DXExt.h を追加 
	* 必要に応じて DXImage.h, DXRT.h, DXMS.h
 * Source Files に Win.cpp, DX.cpp, DXExt.cpp を追加
 	* 必要に応じて DXImage.cpp
 * XxxDX.h、XxxDX.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)

 * Shader Files フォルダを作成し、シェーダを突っ込む
  * シェーダファイルを右クリック - プロパティ - Configuration Propeties - General
   * Excluded From Build を No
   * Content を Yes
  * 右クリック - プロパティ - HLSL Compiler - General - Shader Type でタイプを適切に選択しておく
	* RayTracing の場合は Library(/lib) を指定、Entrypoint Name は空欄としておく

* ルートシグネチャ用HLSL XxxDX.rs.hlsl を作成する
* WinPixEventRuntimeのインストール
	* 右クリック - Manage NuGet Packages - Browse - WinPixEventRuntime で検索 - インストール

#### VK
 * プロパティマネージャで Add Existing Property Sheet... - Props/VK_DEBUG.props, Props/VK_RELEASE.props もしくはテクスチャが必要な場合は Props/VK_DEBUG_TEX.prop, Props/VK_RELEASE_TEX.prop
	* 必要に応じて Props/GLTF.prop, Props/HOLO.prop, Props/LEAP.prop, Props/FBX.prop, Props/DRACO.prop, Prop/DXMESH.prop 等を追加
 
 * Header Files に Win.h, VK.h, VKExt.h を追加
	* 必要に応じて VKImage.h, VKRT.h, VKMS.h
 * Source Files に Win.cpp, VK.cpp, VKExt.cpp を追加
	* 必要に応じて VKImage.cpp
 * XxxVK.h、XxxVK.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)

 * Shader Files フォルダを作成し、シェーダを突っ込む
  * 拡張子を glslangValidator に沿うようにしておく
	* .vert, .tesc. .tese, .geom, .frag, .comp, .mesh, .task, .rgen, .rhit, .rahit, .rchit, .rmiss, .rcall,...
  * 右クリック - プロパティ - Configuration Propeties - General
    * Excluded From Build を No
    * Content を Yes
    * Item Type を Custom Build Tool
   * 適用 - Custom Build Tool 項目が追加されるので GLSL Compiler になっていることを確認 (↑のプロパティを先に設定しておくこと)
 -->

<!-- 
# 外部ライブラリ

## GLTF
 * チュートリアル https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/README.md
 * サンプルデータ https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0
 * C++ローダー https://github.com/jessey-git/fx-gltf
 * JSON https://github.com/nlohmann/json
 * DXローダー(使用していない) https://github.com/microsoft/glTF-SDK

## ParametricSurface
 * http://www.3d-meier.de/tut3/Seite0.html

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
# TODO

## VK
	* ストレージバッファ、ユニフォームテクセルバッファ、ストレージテクセルバッファの検証
	* サブパスの検証
## DX
	* コマンドリスト、グラフィクスコマンドリストまわりをまとめる
## VK, DX
	* 深度を使うパターン良く出てくるので共通処理を関数化する？
	* コンスタント(ユニフォーム)バッファをフレーム分用意する(#pragma region FRAME_OBJECT)
		* 未 Deferred
	* ウインドウサイズ変更時の処理 OnSize() スワップチェインのリサイズ
	* コンピュートの検証(テクスチャを準備する)
	* テクスチャ
		* プロシージャルテクスチャ
	* Gバッファ
		* シャドウマップ
		* SSAO
	* C++20
		* constexprの積極的使用
			* コンパイル時に決定するもの
		* string_viewの積極的使用
		* 初期化付if, forの積極的使用
		* std::optional
			~~~
			std::optional<int> a = 1;
			if(a.has_value()) { a.value(); }
			else { a.reset(); }
			std::cout << a.value_or(-1) << std::endl;
			~~~
		* std::variant
			* union代用
		* std::format
			* まだコンパイラが対応してない
		* 
			~~~
			//template<typename T> requires is_integral_v<T> 
			//T mod(T lhs, T rhs){ return lhs % rhs; }
			template<integral T>
			T mod(T lhs, T rhs){ return lhs % rhs; }
			//template<typename T> requires is_floating_point_v<T> 
			//T mod(T lhs, T rhs){ return fmod(lhs, rhs); }
			template<floating_point T>
			T mod(T lhs, T rhs){ return fmod(lhs, rhs); }
			~~~

## 保留(要ハードウェア)
	* HDR
	* メッシュシェーダ
	* レイトレーシング
		* 拡張
			VK_KHR_acceleration_structure
			VK_KHR_ray_tracing_pipeline
			VK_KHR_ray_query
			VK_KHR_pipeline_library
			VK_KHR_deferred_host_operations
		* Acceleration Structure (AS) ... VK_KHR_acceleration_structure
			* vkCmdBuildAccelerationStructuresKHR(), VkAccelerationStructureBuildGeometryInfoKHR で作成 
			* 更新が必要な場合 .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR、srcAccelerationStructure, dstAccelerationStructure で指定
			* Bottom Level AS (BLAS)
			* Top Level AS (TLAS)
		* シェーダステージ ... VK_KHR_ray_tracing_pipeline
			* Ray Generation Shader (RGS)
			* Closest Hit Shader (CHS)
			* Miss Shader
			* Intersection Shader
			* Any-hit Shader

			* RGS, CHS はマテリアル、テクスチャデータ等の参照を必要とする
			* ペイロード 
				* 5つのシェーダステージ間のコミュニケーションに使われる、どのようにヒット、ミスを扱うか
				* rayPayloadEXT 

	* マルチGPUの場合の処理
 --> 

<!--
# 圧縮テクスチャメモ
	DXT1	... BC1		bpp4	RGB,RGBA	A2諧調
	DXT2,3	...	BC2		bpp8	RGBA		A16諧調
	DXT4,5	... BC3		bpp8	RGBA
	ATI1N	... BC4		bpp4	R			ハイトマップ等
	ATI2N	... BC5		bpp8	RG			ノーマルマップ等
				BC6H	bpp8	RGB			HDR
				BC7		bpp8	RGB,RGBA
-->