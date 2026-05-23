import json
import sys
from datetime import datetime
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: demo_solver.py <case.json> <result.json>", file=sys.stderr)
        return 2

    case_path = Path(sys.argv[1])
    result_path = Path(sys.argv[2])

    if not case_path.exists():
        print(f"case file does not exist: {case_path}", file=sys.stderr)
        return 3

    with case_path.open("r", encoding="utf-8-sig") as case_file:
        case_data = json.load(case_file)

    materials = case_data.get("materials", [])
    boundary_conditions = case_data.get("boundaryConditions", [])
    loads = case_data.get("loads", [])

    result = {
        "solverId": "demo",
        "solverName": "External Demo Solver",
        "status": "completed",
        "finishedAt": datetime.now().isoformat(timespec="seconds"),
        "caseId": case_data.get("id", ""),
        "caseName": case_data.get("name", ""),
        "summary": "External Python demo solver completed. No physical equation was solved.",
        "inputCounts": {
            "materials": len(materials),
            "boundaryConditions": len(boundary_conditions),
            "loads": len(loads),
        },
        "iterations": 1,
        "maxResidual": 0.0,
    }

    result_path.parent.mkdir(parents=True, exist_ok=True)
    with result_path.open("w", encoding="utf-8") as result_file:
        json.dump(result, result_file, indent=4)

    print(f"demo solver wrote {result_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
