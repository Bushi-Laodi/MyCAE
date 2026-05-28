$ErrorActionPreference = "Stop"
$serviceRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $serviceRoot
python -m uvicorn openfoam_demo_service:app --host 127.0.0.1 --port 8765
