set PYTHONPATH=%~dp0python
python3 "%PYTHONPATH%/Trim/Trim.py" %* || exit /b 1
