add_library(WindHorn STATIC 
    RegHandler.cpp
    StdAfx.cpp
    WHDefProcess.cpp
    WHDXGraphic.cpp
    WHImage.cpp
    WHSurface.cpp
    WHWilImage.cpp
    WHWilTexture.cpp
    WHWindow.cpp
    D3DCommon/d3dmath.cpp
    D3DCommon/d3dtextr.cpp
    D3DCommon/d3dutil.cpp
    D3DCommon/ddutil.cpp
    # WH3DInit.cpp
)

target_compile_definitions(WindHorn PRIVATE _LIB)
target_include_directories(WindHorn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(WindHorn PUBLIC ddraw d3d9 dsound dxguid winmm)
