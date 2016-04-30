# VKDX

## VK

* SDK
    * https://vulkan.lunarg.com/signin
    * インストールすると環境変数 **VK_SDK_PATH** が自動的に作成される
    
* ドライバ
    * https://www.khronos.org/vulkan/

* GLM
    * https://github.com/g-truc/glm
    * 環境変数 **GLM_SDK_PATH** を作成した
* GLI
    * https://github.com/g-truc/gli
    * 環境変数 **GLI_SDK_PATH** を作成した

* シェーダコンパイル
    * glslangValidator.exe でコンパイルする 環境変数 **Path** が通っているらしくそのまま使用できる
    * BuildEvent - Post-Build Event に以下のように指定した 
    
    `for %%1 in (*.vert, *.tesc, *.tese, *.geom, *.frag, *.comp) do glslangValidator -V %%1 -o %%1.spv`
    
## DX