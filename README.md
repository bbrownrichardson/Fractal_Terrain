# Fractal_Terrain
Final project for computer graphics course

Contributors: Brianna Richardson, Blaire Bosley, and Tristan Lopus

Programs generates terrain with different options in a skybox.

* Computer Graphics - Project 4

** Controls
- R: load a new terrain
- V: toggle between first-person nav mode and observer model view mode
- W/S: in first-person nav mode, move forward/backward
- P: in first-person nav mode, reset to original position
- F: toggle fog on/off
- +/-: increase/decrease fog density

** Navigation
- In first person nav mode:
  - W/S: move forward/backward
  - Move mouse: rotate view perspective
  - Click and drag mouse: reposition mouse without rotating view
- In observer model view mode:
  - Click and drag mouse: rotate terrain

** Menus
  - Appearance Mode
    - Color: shows colored terrain
    - Texture: shows textured terrain
    - Texture and Color: shows terrain with color and texture
    - Diamond Square Visualization: originally a mistake during texturing, uses the i, j grid coordinates as texture coordinates, such that the textures end up offering a visual representation of the "diamonds" and "squares" of the diamond square algorithm
  - Toggle Water
    - Water on: terrain at or below y=0.3 is flattened at y=0.3 and given water texture
    - Water off: terrain below y=0.3 is given grass texture

