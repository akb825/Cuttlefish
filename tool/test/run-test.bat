@ECHO OFF
set command=%1
set command=%command:/=\%
set args=%~2
set exitCode=%3

"%command%" %args%
set realExitCode=%ERRORLEVEL%
if %exitCode% NEQ %realExitCode% (
	echo got exit code %realExitCode%; expected %exitCode%
	exit /b -1
)
