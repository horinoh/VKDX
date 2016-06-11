# VKDX

## VK

#### SDK
* https://vulkan.lunarg.com/signin
* インストールすると環境変数 **VK_SDK_PATH** が自動的に作成される
	* 新しいバージョンをインストールしたら(環境変数は維持されるが)パスが変わるので VAssistX - Visual Assist Options - Performance - Rebuild しておく
* Visual Stuido で C/C++ - Preprocessor - Preprocessor Definitions に **VK_USE_PLATFORM_WIN32_KHR** を定義した

#### ドライバ
* https://www.khronos.org/vulkan/
	* NVIDIA のドライバアップデート後に vkCreateInstance() でコケるようになったら、恐らく Vulkan ドライバを再インストールすると治る

#### GLM
* https://github.com/g-truc/glm
* 同じ階層に GLM をクローンして **..\..\glm** にパスを通した

#### GLI
* https://github.com/g-truc/gli
* 同じ階層に GLI をクローンして **..\..\gli** にパスを通した

#### シェーダコンパイル
* glslangValidator.exe でコンパイルする 環境変数 **Path** が通っているらしくそのまま使用できる
* Visual Studio で BuildEvent - Post-Build Event に以下のように指定した 
~~~
for %%1 in (*.vert, *.tesc, *.tese, *.geom, *.frag, *.comp) do glslangValidator -V %%1 -o $(ProjectDir)%%1.spv
~~~
    
## DX

#### Visual Studio
 * Visual Studio のインストール時に Universal Windows App Development Tools - Tools and Windows 10 SDK 10.XXX をチェックしておく必要がある
 * インストール済みの場合は「プログラムと機能」から更新インストールする 

#### シェーダコンパイル
 * シェーダは Visual Studio に追加すると自動的にコンパイルされる
 * Properties - HLSLCompiler - General - Shader Type を適切に設定しておかないと、頂点シェーダでコンパイルされるので注意
 * HLSL Compiler - Output Files - $(OutDir)%(Filename).cso を $(ProjectDir)%(Filename).cso へ変更した

#### デバッグ
 * 参考 https://msdn.microsoft.com/ja-jp/library/hh873204.aspx
	* Alt + F5 で開始 (Debug - Graphics - Start Graphics Debugging)
	* PrintScreen でキャプチャ (Debug - Graphics - Capture Frame)
	* キャプチャしたフレームがサムネイルされる、ダブルクリックすると Analyzer が起動する
		* 下の方に出るので Frame time, Frames per second を閉じないと見えないかも

## DDS ツール
* https://directxtex.codeplex.com/wikipage?title=Texconv&referringTitle=Documentation
* https://directxtex.codeplex.com/wikipage?title=Texassemble&referringTitle=Documentation

<!-- 
## プロジェクトの追加方法 (自分用覚書)
 * ソリューションを右クリック - Add - New Project で Win32 Project
 * プロジェクトを右クリック - Retarget SDK Verson で 10以上にする

#### DX
 * プロパティマネージャで Add Existing Property Sheet... - Props/HLSL.props
 * Header Files に Win.h、DX.h を追加 
 * Source Files に Win.cpp、DX.cpp を追加
 * XxxDX.h、XxxDX.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)
 * 必要に応じて Shader Files フォルダを作成し、シェーダを突っ込む
  * 右クリック - プロパティ - HLSL Compiler - General - Shader Type でタイプを適切に選択しておく

#### VK
 * プロパティマネージャで Add Existing Property Sheet... - Props/GLSL.props
 * Header Files に Win.h、VK.h を追加
 * Source Files に Win.cpp、VK.cpp を追加
 * XxxVK.h、XxxVK.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)
 * 必要に応じて Shader Files フォルダを作成し、シェーダを突っ込む
  * 拡張子を glslangValidator に沿うようにタイプを選択しておく。(VS.vert、 PS.frag、...)
 -->

<!-- 
## FBX

 * 環境変数 **FBX_SDK_PATH** を定義しておく
 * 環境変数 **Path** に DLL のパスを通しておく
 ~~~
 Path=%Path%:%FBX_SDK_PATH%\lib\vs2015\x64\debug
 Path=%Path%:%FBX_SDK_PATH%\lib\vs2015\x64\release
 ~~~
 
 ## OPENCV
 
 * 環境変数 **OPENCV_SDK_PATH** を定義しておく
 * 環境変数 **Path** に DLL のパスを通しておく
 ~~~
 Path=%Path%:%OPENCV_SDK_PATH%\x64\vc14\bin
 ~~~
 -->

<!--
TODO

* GLSL のコンパイルのカスタムビルドを作る？
* よく使うパターンは DX.h, VK.h へ持たせたい

* ビルボード
* インスタンシング
* GSインスタンシング
* コンピュート
* パラメトリックサーフェス
* ポストプロセス
* フラットシェーディング
* プロシージャルテクスチャ

* テクスチャ
	* CUBEマップ
	* ディスプレースメント

* FBX
	* アニメーション

* Gバッファ
	* シャドウマップ
	* SSAO
 --> 