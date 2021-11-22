REM texconv は DirectXTex にあるので、予めビルドして、環境変数 Path を通しておく必要がある
@for %%i in (*.jpg) do @texconv %%i