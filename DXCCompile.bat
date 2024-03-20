@rem Usage
@rem $DXCCompile.bat XXX.hlsl

@rem Without Debug info
dxc.exe -E main -T lib_6_8 -Fo %~n1.sco %1

@rem With Debug info
@rem @dxc.exe -E main -T lib_6_8 -Fo %~n1.sco %1 -Zi -Qembed_debug

