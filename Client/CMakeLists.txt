cmake_minimum_required(VERSION 4.3.1)

project(LegendOfMir3_Client VERSION 0.1 LANGUAGES C CXX)

add_subdirectory(WindHorn)

set(MIR3_CLIENT_SOURCES
    Common/Button.cpp
    Common/ChatEditBox.cpp
    Common/ClientSocket.cpp
    Common/DblList.cpp
    Common/DLinkedList.cpp
    Common/EnDecode.cpp
    Common/ImageHandler.cpp
    Common/MirButton.cpp
    Common/MirMsgBox.cpp
    Common/MirMsgBox3D.cpp
    Common/MsgBox.cpp
    Common/NoticeBox.cpp
    Common/NPCTxtAnalysis.cpp
    Common/Queue.cpp

    LoginProcess/LoginProcess.cpp
    LoginProcess/Patch.cpp
    LoginProcess/Common/BMWnd.cpp
    LoginProcess/Intro/LoginAvi.cpp
    LoginProcess/Login/Login.cpp
    "LoginProcess/New Account/NewAccount.cpp"
    "LoginProcess/Select Srv/SelectSrv.cpp"
    # LoginProcess/Change Pwd/ChgPwd.cpp

    CharSelectProcess/CharacterProcess.cpp
    CharSelectProcess/BackGround/BackGround.cpp
    CharSelectProcess/CreateChr/CreateChr.cpp
    CharSelectProcess/SelectChr/SelectChr.cpp

    GameProcess/Actor.cpp
    GameProcess/ChatPopWnd.cpp
    GameProcess/ClientSysMsg.cpp
    GameProcess/ExchangeWnd.cpp
    GameProcess/GameBtn.cpp
    GameProcess/GameProc.cpp
    GameProcess/GameWnd.cpp
    GameProcess/GroupPopWnd.cpp
    GameProcess/GroupWnd.cpp
    GameProcess/GuildMasterWnd.cpp
    GameProcess/GuildWnd.cpp
    GameProcess/HorseWnd.cpp
    GameProcess/Interface.cpp
    GameProcess/InventoryWnd.cpp
    GameProcess/Item.cpp
    GameProcess/LightFog.cpp
    GameProcess/Magic.cpp
    GameProcess/MapHandler.cpp
    GameProcess/NPCWnd.cpp
    GameProcess/OptionWnd.cpp
    GameProcess/Particle.cpp
    GameProcess/QuestWnd.cpp
    GameProcess/SprDfn.cpp
    GameProcess/StatusWnd.cpp
    GameProcess/StoreWnd.cpp
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

add_executable(Mir3CuiClient WIN32 CuiClient.cpp Dialogs.cpp CuiAudio.cpp cui.c Common/EnDecode.cpp)
target_sources(Mir3CuiClient PRIVATE
    CuiLegacy/hud/chat_history.cpp
    CuiLegacy/resources/legacy_movie.cpp
    CuiLegacy/resources/wil_archive.cpp
    CuiLegacy/resources/messages.cpp
    CuiLegacy/text/native_text.cpp
)
target_compile_features(Mir3CuiClient PRIVATE cxx_std_20)
target_include_directories(Mir3CuiClient PRIVATE CuiLegacy)
set_target_properties(Mir3CuiClient PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
set_property(SOURCE cui.c APPEND PROPERTY COMPILE_OPTIONS /utf-8)
target_link_libraries(Mir3CuiClient PRIVATE ws2_32 opengl32 gdi32 user32 imm32 comdlg32 shell32 ole32 xaudio2 vfw32)
