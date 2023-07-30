# プロジェクトの追加方法
 * Windows Desktop Application を選択
 * Source Files に framework.cpp(内容空で作成)を追加
 	* framewordk.cpp に対してのみ C/C++ - Precompiled headers - Precompiled Header を Create(/Yc) にする
	* Precompiled Header File には stdafx.h を消して framework.h と書く
 * framework.h(旧stdafx.h)は既存のものを参考に編集 (#pragma region Code でマークしてある)

## DX
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
	* RayTracing の場合は Library(/lib) を指定、Entrypoint Name は空欄とすること

* ルートシグネチャ用HLSL XxxDX.rs.hlsl を作成する
* WinPixEventRuntimeのインストール
	* 右クリック - Manage NuGet Packages - Browse - WinPixEventRuntime で検索 - インストール

## VK
 * プロパティマネージャで Add Existing Property Sheet... - Props/VK_DEBUG.props, Props/VK_RELEASE.props もしくはテクスチャが必要な場合は Props/VK_DEBUG_TEX.prop, Props/VK_RELEASE_TEX.prop
	* 必要に応じて Props/GLTF.prop, Props/HOLO.prop, Props/LEAP.prop, Props/FBX.prop, Props/DRACO.prop, Prop/DXMESH.prop 等を追加
 
 * Header Files に Win.h, VK.h, VKExt.h を追加
	* 必要に応じて VKImage.h, VKRT.h, VKMS.h
 * Source Files に Win.cpp, VK.cpp, VKExt.cpp, VKDebugUtils.cpp を追加
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