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
for %%1 in (*.vert, *.tesc, *.tese, *.geom, *.frag, *.comp) do glslangValidator -V %%1 -o %%1.spv
~~~
    
## DX

#### Visual Studio
 * Visual Studio のインストール時に Universal Windows App Development Tools - Tools and Windows 10 SDK 10.XXX をチェックしておく必要がある
 * インストール済みの場合は「プログラムと機能」から更新インストールする 

#### シェーダコンパイル
 * シェーダは Visual Studio に追加すると自動的にコンパイルされる]
 * Properties - HLSLCompiler - General - Shader Type を適切に設定しておかないと、頂点シェーダでコンパイルされるので注意

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