# テクスチャ
## DDS
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
## KTX
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

# 圧縮テクスチャメモ

||||||
|-|-|-|-|-|
|DXT1|BC1|bpp4|RGB, RGBA|A2諧調|
|DXT2, 3|BC2|bpp8|RGBA|A16諧調|
|DXT4, 5|BC3|bpp8|RGBA||
|ATI1N|BC4|bpp4|R|ハイトマップ等|
|ATI2N|BC5|bpp8|RG|ノーマルマップ等|
||BC6H|bpp8|RGB|HDR|
||BC7|bpp8|RGB, RGBA||
