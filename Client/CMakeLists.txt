cmake_minimum_required(VERSION 4.3.1)

project(LegendOfMir3_Client VERSION 0.1 LANGUAGES C CXX)

add_subdirectory(WindHorn)

set(MIR3_CLIENT_SOURCES
    Dialogs.cpp
    Common/Button.cpp
    Common/ChatEditBox.cpp
    Common/ClientSocket.cpp
    Common/DblList.cpp
    Common/DLinkedList.cpp
    Common/EnDecode.cpp
    Common/ImageHandler.cpp
    Common/MirButton.cpp
    Common/NPCTxtAnalysis.cpp
    Common/Queue.cpp

    LoginProcess/LoginProcess.cpp
    LoginProcess/Patch.cpp
    # LoginProcess/Change Pwd/ChgPwd.cpp

    CharSelectProcess/CharacterProcess.cpp

    GameProcess/Actor.cpp
    GameProcess/ClientSysMsg.cpp
    GameProcess/GameBtn.cpp
    GameProcess/GameProc.cpp
    GameProcess/Item.cpp
    GameProcess/LightFog.cpp
    GameProcess/Magic.cpp
    GameProcess/MapHandler.cpp
    GameProcess/Particle.cpp
    GameProcess/SprDfn.cpp
    GameProcess/WeatherEffect.cpp
    # GameProcess/PlayerChar.cpp
    # GameProcess/SpriteInfo.cpp
    # GameProcess/Water.cpp

    Sound/BMMP3.cpp
    Sound/MirSound.cpp
    Sound/SBuffer.cpp
    Sound/Sound.cpp
    AVI/Avi.cpp

    Mir2Ex.cpp
    GFun.cpp
    StdAfx.cpp
    Resource.rc
    # ServerSelectProcess/SvrSelProc.cpp
)

add_executable(Client WIN32 ${MIR3_CLIENT_SOURCES})
target_compile_definitions(Client PRIVATE MIR3_LEGACY_CLIENT)

# 现代外观：清单已在 Resource.rc 里以 1 RT_MANIFEST "../app.manifest" 嵌入，
# 这里关闭链接器自动生成的清单，避免同一个 exe 里出现两份 RT_MANIFEST 资源。
target_link_options(Client PRIVATE "/MANIFEST:NO")
set_target_properties(Client PROPERTIES OUTPUT_NAME Mir2Ex)
target_link_libraries(Client PRIVATE WindHorn)

target_include_directories(Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/WindHorn
    ${CMAKE_CURRENT_SOURCE_DIR}/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/GameProcess
    ${CMAKE_CURRENT_SOURCE_DIR}/LoginProcess
    ${CMAKE_CURRENT_SOURCE_DIR}/CharSelectProcess
    ${CMAKE_CURRENT_SOURCE_DIR}/Sound
    ${CMAKE_CURRENT_SOURCE_DIR}/AVI
)

set(MIR3_CUI_UI_SOURCES
    Dialogs.cpp
    CuiAudio.cpp
    cui.c
    Common/EnDecode.cpp
    CuiLegacy/resources/legacy_movie.cpp
    CuiLegacy/resources/wil_archive.cpp
    CuiLegacy/resources/messages.cpp
    CuiLegacy/text/native_text.cpp
)
add_executable(Mir3CuiClient WIN32 Mir3.cpp ${MIR3_CUI_UI_SOURCES})
add_executable(Mir3CuiDialogSmoke EXCLUDE_FROM_ALL Tests/CuiDialogSmoke.cpp ${MIR3_CUI_UI_SOURCES})
set_property(SOURCE cui.c APPEND PROPERTY COMPILE_OPTIONS /utf-8)
set_property(SOURCE Mir3.cpp CuiAudio.cpp Tests/CuiDialogSmoke.cpp APPEND PROPERTY COMPILE_OPTIONS /utf-8 /wd4828)
set_property(SOURCE Dialogs.cpp APPEND PROPERTY COMPILE_OPTIONS
    /wd4828 "$<$<NOT:$<BOOL:$<TARGET_PROPERTY:MIR3_LEGACY_DIALOGS>>>:/utf-8>")
set_property(TARGET Client PROPERTY MIR3_LEGACY_DIALOGS TRUE)
foreach(cui_target Mir3CuiClient Mir3CuiDialogSmoke)
    target_compile_features(${cui_target} PRIVATE cxx_std_20)
    target_include_directories(${cui_target} PRIVATE CuiLegacy)
    set_target_properties(${cui_target} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
    target_link_libraries(${cui_target} PRIVATE ws2_32 opengl32 gdi32 user32 imm32 comdlg32 shell32 ole32 xaudio2 vfw32)
endforeach()
