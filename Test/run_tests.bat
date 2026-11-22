@echo off
setlocal
pushd "%~dp0.." || exit /b 1
set "PYTHONIOENCODING=utf-8"
set "LOG_DIR=out\excel-tests"
if not defined MIR3_TEST_PORT_OFFSET set "MIR3_TEST_PORT_OFFSET=20000"
if "%~1"=="" goto suite

rem Pass a workbook and case, plus any xlsx_scenario.py options.
python Test\xlsx_scenario.py %*
goto finish

:suite
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" || goto failed
python Test\verify_runner.py || goto failed
for %%P in (
    Login/Login
    Login/LoginDirect
    Login/LoginDirectClient2
    Login/LoginDirectClient3
    Character/Character
    Chat/ChatMapIsolation
    Combat/CombatMelee
    GameplaySmoke/GameplaySmoke
    Inventory/Inventory
    Monsters/MonsterAggro
    Npc/NpcDialog
    Shop/ShopBuy
    Skills/SkillCast
) do (
    for /f "tokens=1,2 delims=/" %%A in ("%%P") do (
        echo Running %%A/%%B
        python Test\xlsx_scenario.py Test\%%A.xlsx --case %%B --port-offset %MIR3_TEST_PORT_OFFSET% --log "%LOG_DIR%\%%B.log" >"%LOG_DIR%\%%B.launcher.log" 2>&1 || (
            type "%LOG_DIR%\%%B.launcher.log"
            goto failed
        )
    )
)
echo Passed. Logs: "%CD%\%LOG_DIR%"
goto finish

:failed
echo Test suite failed. Logs: "%CD%\%LOG_DIR%" 1>&2
popd
exit /b 1

:finish
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
