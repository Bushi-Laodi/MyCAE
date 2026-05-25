# MyCAE Demo Acceptance Checklist

Use this checklist when showing the current MyCAE CalculiX workflow.

## Project Loading

- [ ] Open `samples/projects/box_pressure_demo/project.json`.
- [ ] Project name is `Box Pressure Demo`.
- [ ] Project tree contains one geometry object, one mesh object, one simulation case, and one CalculiX result.
- [ ] The resource manager reports no missing geometry, mesh, solver, or result files.

## Geometry And Mesh

- [ ] Display `Box_1`.
- [ ] Display `Box_1_Mesh`.
- [ ] Mesh metadata shows 111 nodes and 264 tetrahedra.
- [ ] Face groups are available for `PickedFaces1` and `LoadEnd`.

## Simulation Case

- [ ] Simulation case loads successfully.
- [ ] Material count is 1.
- [ ] Boundary condition count is 2.
- [ ] Load count is 1.
- [ ] Exporting CalculiX input creates a valid `.inp` deck.

## Result Viewing

- [ ] Select `CalculiX Result - Box Pressure Demo`.
- [ ] Display `Displacement Magnitude`.
- [ ] Switch to `Von Mises Stress`.
- [ ] Toggle mesh edges.
- [ ] Toggle undeformed overlay.
- [ ] Change deformation scale and confirm the view updates.
- [ ] Export CSV result data.
- [ ] Export Markdown result report.
- [ ] Save a render screenshot.

## Solver Re-run

- [ ] Run CalculiX from the demo project.
- [ ] A new result directory appears under `solver/calculix`.
- [ ] `.dat`, `.frd`, `.sta`, and `.log` files are generated.
- [ ] Solver completion is reported as `yes`.
- [ ] New result can be selected and displayed.
