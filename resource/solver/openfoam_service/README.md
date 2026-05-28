# MyCAE OpenFOAM Demo Service

This service is a pre-research bridge for the future OpenFOAM integration. It does not run a real OpenFOAM solver yet. It accepts a case request from the Qt application, writes a log, and returns a synthetic VTK result file.

## Start

```powershell
cd resource\solver\openfoam_service
python -m pip install -r requirements.txt
python -m uvicorn openfoam_demo_service:app --host 127.0.0.1 --port 8765
```

The Qt side uses `http://127.0.0.1:8765` by default. Override it with:

```powershell
$env:MYCAE_OPENFOAM_SERVICE_URL = "http://127.0.0.1:8765"
```

## Endpoints

- `GET /health`
- `POST /run`

`POST /run` creates:

- `openfoam_demo.log`
- `openfoam_service_request.json`
- `postProcessing/vtk/<case>_openfoam_demo.vtk`
