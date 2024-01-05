# VKDX

## VK、DX 共通

### Visual Studio
* プロジェクト右クリック - C/C++ - Language - C++ Language Standard - ISO C++20 Standard (/std:c++20) を選択

### コンパイル
* "warning C4005: '_malloca': macro redefinition" は framework.h 内 windows.h 前に _CRTDBG_MAP_ALLOC を定義すると出なくなる
	~~~
	#define _CRTDBG_MAP_ALLOC
	#include <windows.h>
	~~~
### プリコンパイルヘッダーを使う
 * (ここでは)framework.cpp を追加 - 右クリック - Properties - C/C++ - Precompiled Headers - Create(/Yc)
 * Properties - C/C++ - Advanced - Forced Include File に framework.h を指定しているので、framework.cpp に #include "framework.h" は記述しなくて良い

### Gitサブモジュール
 * TortoiseGit - Submodule update - 対象(Path)を選択 - Remote tracking branch にチェックを入れて - OK でサブモジュールを最新のものに更新できる

### Visual Assist X
* 検索対象に拡張子(inl)を追加する
	* Tool - Options - Text Editor - File Extension - Editor で Microsoft Visual C++ を選択 - Extension に inl を記述 - Add
	* Visual Assist X をリビルドする
	
### 素材
* Thanks to these asetts
	* [cc0textures](https://cc0textures.com/)
	* [polyhaven](https://polyhaven.com/)
	* [The Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/)

### [テクスチャ](https://github.com/horinoh/VKDX/tree/master/DocTexture)

### [外部ライブラリ](https://github.com/horinoh/VKDX/tree/master/DocExternal)
- GLTF, FBX, DRACO
- DirectXMesh
- LeapMotion

## VK

### [SDK](https://vulkan.lunarg.com/signin)
* インストールすると環境変数 **VK_SDK_PATH**、**VULKAN_SDK** が自動的に作成される
	* VAX : 新しいバージョンをインストールしたらパスが変わるので VAssistX - Visual Assist Options - Performance - Rebuild しておく
	* ~~UE4 : 環境変数 **VULKAN_SDK** は UE4 のコンパイルが通らなくなるので消した `setx VULKAN_SDK ""`~~
* 以下のように定義しておく
	~~~
	#ifdef _WINDOWS
	#define VK_USE_PLATFORM_WIN32_KHR
	#endif
	~~~

* DLL
	* C/C++ - Preprocessor - Preprocessor Definitions に **VK_NO_PROTOYYPES** を定義しておく
	* %VK_SDK_PATH%\RunTimeInstaller\VulkanRT-XXX-Installer.exe を実行すると DLL がインストールされる
		* SDKのインストール時に自動的に行われている？

* バリデーションレイヤ
	* アプリ と VulkanAPI の間のレイヤ
	
### [ドライバ](https://www.khronos.org/vulkan/)
* NVIDIA のドライバアップデート後に vkCreateInstance() でコケるようになったら、恐らく Vulkan ドライバを再インストールすると治る

### [GLM](https://github.com/g-truc/glm)
* サブモジュール化した **..\\..\glm** へインクルードパスを通した
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
### [GLI](https://github.com/g-truc/gli)
* サブモジュール化した **..\\..\gli** へインクルードパスを通した
	* .gitmoudles に以下を追加
		~~~
		ignore = dirty
		~~~

### [Vulkan-Hpp(未使用)](https://github.com/KhronosGroup/Vulkan-Hpp)

### シェーダコンパイル
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

### オンラインコンパイル
* VulkanSDK/バージョン/glslang/CMakeList.txt を使って cmake でプロジェクトを生成
* glslang.sln を開いて glslang, glslangValidator, OGLCompipler, OSDependent, SPIRV, spirv-remap をビルド
	* 64bitで使う場合は x64 を追加してビルドしないとだめみたい
* SPIRV.lib, glslang.lib, OGLCompiler.lib, OSDependent.lib
	* ここでは未使用

### Visual Studio で GLSL, HLSL シンタックスハイライトさせる場合
* Visual Studio - Extensions - Manage Extensions
	* GLSL language integration 拡張を検索してインストールする
	* HLSL Tools for Visual Studio 拡張を検索してインストールする

### デバッグ
* [RenderDoc](https://renderdoc.org/builds)
	* RenderDoc 無しで、Debug 版をビルドするには、USE_RENDERDOC の定義をやめる
	* [参考](http://www.saschawillems.de/?page_id=2017)
		* RenderDocをインストールして起動 - Warning をクリックすると Windows のレジストリが作られる(初回のみ)
		* Executable Path に exe を指定して、 Launch ボタンでアプリを実行、PrintScreen でシーンをキャプチャしてアプリを閉じる
			* 必要に応じて上記のexe指定を cap ファイルに保存しておく
		* **RenderDoc から実行した場合にしか VK_EXT_DEBUG_MARKER_EXTENSION_NAME は有効にならないみたいなので注意**
* [Nsight](https://developer.nvidia.com/nsight-graphics)

## DX

### Visual Studio
 * Visual Studio のインストール時に Universal Windows App Development Tools - Tools and Windows 10 SDK 10.XXX をチェックしておく必要がある
 * インストール済みの場合は「プログラムと機能」から更新インストールする 

### WinRt
* [Microsoft::WRL::ComPtr から winrt::com_ptr への移行](https://docs.microsoft.com/ja-jp/windows/uwp/cpp-and-winrt-apis/move-to-winrt-from-wrl)
	* 以下のようなコード変更を行う
		|||
		|-|-|
		|#include <wrl.h>|#include <winrt/base.h>|
		|Microsoft::WRL::ComPtr<ID3D12Device> Dev|winrt::com_ptr<ID3D12Device> Dev|
		|XXX.Get()|XXX.get()|
		|&XXX|XXX.put()|
		|XXX.GetAddressOf()|XXX.put_void()|
		|XXX.Reset()|XXX = nullptr|
		|XXX.ReleaseAndGetAddressOf()|XXX = nullptr; XXX.put_void()|
		|XXX.As(&YYY)|XXX->QueryInterface(__uuidof(YYY), YYY.put_void()); winrt::copy_to_abi(XXX, *YYY.put_void());|
		|IID_PPV_ARGS(&XXX)|__uuidof(XXX), XXX.put_void()|
		|IID_PPV_ARGS(XXX.GetAddressOf())|__uuidof(XXX), XXX.put_void()|
		|IID_PPV_ARGS(XXX.ReleaseAndGetAddressOf())|XXX = nullptr; __uuidof(XXX), XXX.put_void()|

### [DirectXTK](https://github.com/Microsoft/DirectXTK12)
* DirectXTK_Desktop_2022_Win10.sln を開いて x64 をビルド (Debug, Release)
	* D3D12_DESCRIPTOR_RANGE1 がないと言われて、コンパイルが通らない場合は  Windows 10 Anniversary Update SDK が必要 (VisualStudioを更新する)
* サブモジュール化したので再帰的にクローンする
	* .gitmoudles に以下を追加
		~~~
		ignore = dirty
		~~~
* 備考
	* [DirectXTex](https://github.com/Microsoft/DirectXTex/wiki/DirectXTex) もある

### シェーダコンパイル
* シェーダは Visual Studio に追加すると自動的にコンパイルされる
* Properties - HLSLCompiler - General - Shader Type を適切に設定しておかないと、頂点シェーダでコンパイルされるので注意
* HLSL Compiler - Output Files - \$(OutDir)%(Filename).cso を $(ProjectDir)%(Filename).cso へ変更した
* .exe 直起動もできるように TargetDir にもコピーしている
	* Visual Studio で BuildEvent - Post-Build Event に以下のように指定した
		~~~
		for %%1 in (*.cso) do xcopy /y %%1 $(TargetDir) //!< TargetDir にもコピー
		~~~

### デバッグ
* WinPixEventRuntimeのインストール
	* ソリューション右クリック - ソリューションのNuGetパッケージの管理 - 参照タブ - WinPixEventRuntimeで検索 - プロジェクトを選択してインストール
		* これをすると #include <pix3.h> が可能になる

 * [参考](https://msdn.microsoft.com/ja-jp/library/hh873204.aspx)
	* Alt + F5 で開始 (Debug - Graphics - Start Graphics Debugging)
	* PrintScreen でキャプチャ (Debug - Graphics - Capture Frame)
	* キャプチャしたフレームがサムネイルされる、ダブルクリックすると Analyzer が起動する
		* 下の方に出るので Frame time, Frames per second を閉じないと見えないかも

### ルートシグネチャをHLSLで指定する
* [参考](https://docs.microsoft.com/en-us/windows/desktop/direct3d12/specifying-root-signatures-in-hlsl)
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

# 他
## 参考
* [パラメトリックサーフェス](http://www.3d-meier.de/tut3/Seite0.html)

## レイトレーシング
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
	
## トラブルシューティング
* 「このプロジェクトは、このコンピュータ上にないNugetパッケージを参照しています」と出る場合
	* まず VS を最新にアップデートする、それでダメなら以下を試す
	* .vcproj の <Target>...</Target> を消す
	* WinPixEventRuntimeのアンインストール、インストールを行う
* *.prop を指定してるのに ～が無いとか言われるとき
	* *.prop を一旦Removeして、再度Addするとうまく行ったりする

## 条件コンパイル
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
		- vkCmdNextSubpass でコマンドを次のサブパスへ進める
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

## [自分用覚書](https://github.com/horinoh/VKDX/tree/master/DocPrivate)

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
