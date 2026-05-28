from __future__ import annotations

import json
import math
import uuid
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field


SERVICE_VERSION = "0.1.0"

app = FastAPI(
    title="MyCAE OpenFOAM Demo Service",
    version=SERVICE_VERSION,
    description="Demo HTTP wrapper that mimics an OpenFOAM run and returns log plus VTK result files.",
)


class OpenFoamRunRequest(BaseModel):
    caseName: str = Field(default="openfoam")
    caseDirectory: str
    projectName: str = Field(default="")
    meshName: str = Field(default="")
    geometryCount: int = Field(default=0, ge=0)
    meshNodeCount: int = Field(default=0, ge=0)
    meshCellCount: int = Field(default=0, ge=0)
    materialCount: int = Field(default=0, ge=0)
    boundaryConditionCount: int = Field(default=0, ge=0)
    loadCount: int = Field(default=0, ge=0)
    metadata: dict[str, Any] = Field(default_factory=dict)


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def ensure_case_directory(case_directory: str) -> Path:
    path = Path(case_directory).expanduser().resolve()
    path.mkdir(parents=True, exist_ok=True)
    return path


def demo_scalar(index: int, count: int) -> float:
    if count <= 1:
        return 0.0
    angle = (2.0 * math.pi * index) / count
    return 0.5 + 0.5 * math.sin(angle)


def write_demo_vtk(path: Path) -> None:
    points = [
        (0.0, 0.0, 0.0),
        (1.0, 0.0, 0.0),
        (1.0, 1.0, 0.0),
        (0.0, 1.0, 0.0),
        (0.0, 0.0, 1.0),
        (1.0, 0.0, 1.0),
        (1.0, 1.0, 1.0),
        (0.0, 1.0, 1.0),
    ]
    cells = [
        (0, 1, 2, 4),
        (2, 3, 0, 7),
        (2, 4, 6, 7),
        (1, 2, 4, 6),
        (2, 5, 1, 6),
    ]

    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write("# vtk DataFile Version 3.0\n")
        stream.write("MyCAE OpenFOAM demo result\n")
        stream.write("ASCII\n")
        stream.write("DATASET UNSTRUCTURED_GRID\n")
        stream.write(f"POINTS {len(points)} float\n")
        for x, y, z in points:
            stream.write(f"{x:.6f} {y:.6f} {z:.6f}\n")
        stream.write(f"CELLS {len(cells)} {len(cells) * 5}\n")
        for cell in cells:
            stream.write("4 " + " ".join(str(i) for i in cell) + "\n")
        stream.write(f"CELL_TYPES {len(cells)}\n")
        stream.write(("10\n" * len(cells)))
        stream.write(f"POINT_DATA {len(points)}\n")
        stream.write("SCALARS pressure float 1\n")
        stream.write("LOOKUP_TABLE default\n")
        for index in range(len(points)):
            stream.write(f"{demo_scalar(index, len(points)):.6f}\n")
        stream.write("VECTORS velocity float\n")
        for x, y, z in points:
            stream.write(f"{0.2 + x * 0.1:.6f} {0.1 + y * 0.1:.6f} {z * 0.05:.6f}\n")


@app.get("/health")
def health() -> dict[str, str]:
    return {
        "status": "ok",
        "service": "mycae-openfoam-demo",
        "version": SERVICE_VERSION,
        "time": utc_now(),
    }


@app.post("/run")
def run_openfoam_demo(request: OpenFoamRunRequest) -> dict[str, Any]:
    try:
        case_dir = ensure_case_directory(request.caseDirectory)
    except OSError as exc:
        raise HTTPException(status_code=400, detail=f"Cannot create case directory: {exc}") from exc

    job_id = str(uuid.uuid4())
    post_dir = case_dir / "postProcessing" / "vtk"
    post_dir.mkdir(parents=True, exist_ok=True)

    vtk_path = post_dir / f"{request.caseName}_openfoam_demo.vtk"
    log_path = case_dir / "openfoam_demo.log"
    manifest_path = case_dir / "openfoam_service_request.json"

    write_demo_vtk(vtk_path)

    log_lines = [
        f"OpenFOAM demo service version: {SERVICE_VERSION}",
        f"Job id: {job_id}",
        f"Case: {request.caseName}",
        f"Project: {request.projectName or '<unnamed>'}",
        f"Mesh: {request.meshName or '<none>'}",
        f"Input summary: geometries={request.geometryCount}, nodes={request.meshNodeCount}, cells={request.meshCellCount}",
        f"Simulation setup: materials={request.materialCount}, boundaries={request.boundaryConditionCount}, loads={request.loadCount}",
        "Demo solver: generated synthetic pressure and velocity fields.",
        f"VTK result: {vtk_path}",
        "OpenFOAM demo completed successfully.",
    ]

    log_path.write_text("\n".join(log_lines) + "\n", encoding="utf-8")
    manifest_path.write_text(
        json.dumps(request.model_dump(), ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    return {
        "success": True,
        "jobId": job_id,
        "summary": "OpenFOAM demo service completed: generated 1 VTK result file.",
        "log": log_lines,
        "logFile": str(log_path),
        "vtkFiles": [str(vtk_path)],
        "caseDirectory": str(case_dir),
        "completedAt": utc_now(),
    }


if __name__ == "__main__":
    import uvicorn

    uvicorn.run("openfoam_demo_service:app", host="127.0.0.1", port=8765, reload=False)
