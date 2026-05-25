# Box Pressure Demo

This is the formal MyCAE acceptance demo project for the current CalculiX workflow.

## Contents

- Geometry: one Open CASCADE box (`Box_1`), 100 mm x 20 mm x 20 mm.
- Mesh: one Gmsh tetrahedral mesh (`Box_1_Mesh`), 111 nodes and 264 tetrahedra.
- Simulation case: one steel material, two face-group boundary conditions, and one pressure load.
- Solver result: one completed CalculiX run with `.inp`, `.dat`, `.frd`, `.sta`, and `.log` files.

## Open

1. Start MyCAE.
2. Open this file:
   `samples/projects/box_pressure_demo/project.json`
3. Confirm that the project tree shows geometry, mesh, simulation case data, and one CalculiX result.

## Expected Result

- Geometry and mesh can be displayed.
- The simulation case loads with 1 material, 2 boundary conditions, and 1 load.
- The CalculiX result is available without running the solver again.
- Available fields include `Displacement Magnitude` and `Von Mises Stress`.
- The result scalar range should include:
  - min: `0`
  - max displacement magnitude: about `1.08369064627412e-05`

## Optional Solver Re-run

If CalculiX is configured, re-run the case from MyCAE. A successful run should create a new directory under:

`solver/calculix/`

The solver log should end with a completed result and the result index should contain the new run.
