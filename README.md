# VKDX

## VK

#### SDK
* https://vulkan.lunarg.com/signin
* インストールすると環境変数 **VK_SDK_PATH** が自動的に作成される
* Visual Stuido で C/C++ - Preprocessor - Preprocessor Definitions に **VK_USE_PLATFORM_WIN32_KHR** を定義した
    
#### ドライバ
* https://www.khronos.org/vulkan/

#### GLM
* https://github.com/g-truc/glm
* ~~環境変数 **GLM_SDK_PATH** を作成した~~ 同じ階層に GLM をクローンして **..\..\glm** にパスを通した

#### GLI
* https://github.com/g-truc/gli
* ~~環境変数 **GLI_SDK_PATH** を作成した~~ 同じ階層に GLI をクローンして **..\..\gli** にパスを通した

#### シェーダコンパイル
* glslangValidator.exe でコンパイルする 環境変数 **Path** が通っているらしくそのまま使用できる
* Visual Studio で BuildEvent - Post-Build Event に以下のように指定した 
~~~
for %%1 in (*.vert, *.tesc, *.tese, *.geom, *.frag, *.comp) do glslangValidator -V %%1 -o $(OutDir)%%1.spv
~~~
    
## DX

#### Visual Studio
 * Visual Studio のインストール時に Universal Windows App Development Tools - Tools and Windows 10 SDK 10.XXX をチェックしておく必要がある
 * インストール済みの場合は「プログラムと機能」から更新インストールする 

#### シェーダコンパイル
 * シェーダは Visual Studio に追加すると自動的にコンパイルされる]
 * Properties - HLSLCompiler - General - Shader Type を適切に設定しておかないと、頂点シェーダでコンパイルされるので注意

## プロジェクトの追加方法 (自分用覚書)
 * ソリューションを右クリック - Add - New Project で Win32 Project
 * プロジェクトを右クリック - Retarget SDK Verson で 10以上にする

### DX
 * Header Files に Win.h、DX.h を追加 
 * Source Files に Win.cpp、DX.cpp を追加
 * XxxDX.h、XxxDX.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)
 * 必要に応じて Shader Files フォルダを作成し、シェーダを突っ込む
  * 右クリック - プロパティ - HLSL Compiler - General - Shader Type でタイプを適切に選択しておく

### VK
 * Header Files に Win.h、VK.h を追加
 * Source Files に Win.cpp、VK.cpp を追加
 * XxxVK.h、XxxVK.cpp は既存のものを参考に編集 (#pragma region Code でマークしてある)
 * 必要に応じて Shader Files フォルダを作成し、シェーダを突っ込む
  * 拡張子を glslangValidator に沿うようにタイプを選択しておく。(VS.vert、 PS.frag、...)

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