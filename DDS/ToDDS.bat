REM texconv は DirectXTex にあるので、予めビルドして、環境変数 Path を通しておく必要がある
REM texconv is in DirectXTex, build and set environment variable PATH, in advance
@for %%i in (*.jpg) do @texconv %%i
@for %%i in (*.png) do @texconv %%i