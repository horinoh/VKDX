@rem UE4が VULKAN_SDK を参照していて UE4 のコンパイルが通らなくなる為削除する
setx VULKAN_SDK ""

@rem setx FBX_SDK_PATH "C:\Program Files\Autodesk\FBX\FBX SDK\2017.0.1"

setx GLM_SDK_PATH "D:\GitHub\glm"
setx GLI_SDK_PATH "D:\GitHub\gli"

setx DXTK_SDK_PATH "D:\GitHub\DirectXTK12"

@pause
