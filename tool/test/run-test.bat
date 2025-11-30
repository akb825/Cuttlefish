@ECHO OFF
set command=%1
set command=%command:/=\%
shift
set exitCode=%1
shift

REM see https://stackoverflow.com/questions/34004969/using-after-shift
set args=
:buildArgs
if @%1==@ goto done
set "args=%args% %1"
shift
goto buildArgs
:done

"%command%" %args%
set realExitCode=%ERRORLEVEL%
if %exitCode% NEQ %realExitCode% (
	echo got exit code %realExitCode%; expected %exitCode%
	exit /b -1
)
