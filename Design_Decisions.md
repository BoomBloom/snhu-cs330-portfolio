# CS 330 Project: 3D Scene Design Decisions Document

**Author:** Mohannad Abuimeish  
**Course:** CS 330: Computational Graphics and Visualization  
**Institution:** Southern New Hampshire University  

---

## 1. Executive Summary & Objective

The objective of this project was to replicate a real-world desk scene deconstructed from a 2D reference photograph into a fully realized, interactive 3D virtual environment. The scene incorporates multiple complex composite objects resting on a planar desk surface, rendered in real time using Modern OpenGL, GLSL shaders, and the C++ standard library.

This document details the architectural, geometric, material, illumination, and interaction design decisions made throughout the project's development cycle.

---

## 2. Geometric Primitives & Spatial Composition

To achieve realistic visual fidelity while maintaining computational efficiency, complex real-world objects were deconstructed into combinations of basic 3D geometric primitives.

```
+------------------------------------------------------------------------+
|                               3D SCENE                                 |
+------------------------------------------------------------------------+
                                   |
         +-------------------------+-------------------------+
         |                         |                         |
   [Desk Surface]           [Coffee Mug]               [Eyeglasses]
    - PlaneMesh              - CylinderMesh (body)      - TorusMesh x2 (frames)
                             - TorusMesh (handle)       - CylinderMesh (bridge)
                             - CylinderMesh (coffee)    - BoxMesh x2 (temples)
                                   |
                             [Keyboard]
                              - BoxMesh (base)
                              - BoxMesh (keybed)
```

### 2.1 Desk Surface
* **Primitive:** `PlaneMesh`
* **Scale Dimensions:** `(20.0f, 1.0f, 14.0f)`
* **World Position:** `(0.0f, 0.0f, 0.0f)`
* **Design Decision:** A large, flat plane forms the world-space ground upon which all other objects reside. By defining the table at $Y = 0.0$, all object elevations can be referenced directly to the table surface, eliminating floating objects or unintentional intersection.

### 2.2 Coffee Cup (Composite Object)
The coffee mug is positioned on the left side of the desk (`anchor: (-4.5f, 0.0f, 1.0f)`) and consists of three distinct components:
1. **Mug Body (`CylinderMesh`):**
   * *Scale:* `(1.25f, 2.5f, 1.25f)`
   * *Position:* Resting flush on desk surface at $Y = 0.0$.
   * *Design Decision:* An upright cylinder represents the main porcelain container, providing smooth cylindrical surface normals for lighting.
2. **Mug Handle (`TorusMesh`):**
   * *Scale:* `(0.75f, 0.75f, 0.75f)`
   * *Rotation:* $90^\circ$ around the Z-axis.
   * *Position:* Offset to `(-1.4f, 1.25f, 0.0f)` relative to the mug center.
   * *Design Decision:* A torus primitive with a cross-section radius of $0.25f$ provides a natural circular loop handle that seamlessly penetrates the side wall of the cylinder.
3. **Coffee Liquid Surface (`CylinderMesh`):**
   * *Scale:* `(1.15f, 0.06f, 1.15f)`
   * *Position:* Offset to `(0.0f, 2.35f, 0.0f)` relative to the mug base.
   * *Design Decision:* A flat, shallow cylinder scaled slightly smaller than the outer rim ($1.15 < 1.25$) sits near the top of the mug, creating the optical illusion of liquid filled close to the brim without clipping through the outer porcelain walls.

### 2.3 Eyeglasses (Composite Object)
The eyeglasses are placed in the right foreground (`anchor: (3.8f, 0.22f, 2.8f)`) and consist of five interconnected components:
1. **Left and Right Lens Frames (`TorusMesh` x2):**
   * *Scale:* `(0.75f, 0.75f, 0.75f)`
   * *Rotation:* $90^\circ$ on the X-axis to lie flat on the desk.
   * *Positions:* Left frame at $X = -0.95f$, right frame at $X = +0.95f$ relative to glasses center.
   * *Design Decision:* Using circular tori provides smooth, rounded metallic rims matching modern circular wireframe glasses.
2. **Nose Bridge (`CylinderMesh`):**
   * *Scale:* `(0.08f, 1.2f, 0.08f)`
   * *Rotation:* $90^\circ$ around the Z-axis to lie horizontally.
   * *Position:* Centered at `(0.0f, 0.0f, 0.0f)` between the two lens rims.
   * *Design Decision:* A thin horizontal cylinder connects the two torus frames at their inner perimeters, ensuring clean structural continuity.
3. **Left and Right Temple Arms (`BoxMesh` x2):**
   * *Scale:* `(0.1f, 0.1f, 2.2f)`
   * *Rotations:* Left arm rotated $-12^\circ$ on the Y-axis; right arm rotated $+12^\circ$ on the Y-axis.
   * *Positions:* Left arm at `(-1.8f, 0.0f, -1.1f)`, right arm at `(+1.8f, 0.0f, -1.1f)` relative to the bridge.
   * *Design Decision:* Slender, elongated box primitives angled outward create realistic glasses arms folded open and resting naturally on the desk plane.

### 2.4 Computer Keyboard (Composite Object)
The keyboard is positioned centered in the upper background of the desk (`anchor: (0.0f, 0.0f, -1.8f)`):
1. **Keyboard Base Shell (`BoxMesh`):**
   * *Scale:* `(9.0f, 0.3f, 3.6f)`
   * *Position:* Centered at $Y = 0.15f$ (half-height), resting directly on the desk plane.
   * *Design Decision:* A wide, low-profile box represents the aluminum unibody chassis of a modern desktop keyboard.
2. **Keybed Cluster Block (`BoxMesh`):**
   * *Scale:* `(8.3f, 0.18f, 3.0f)`
   * *Position:* Elevated at $Y = 0.34f$, sitting atop the keyboard base with a slight border inset.
   * *Design Decision:* Elevating a secondary box above the base creates a distinct geometric bevel and provides an isolated surface for keycap texture mapping.

---

## 3. Transformation Order & Matrix Pipeline

All 3D transformations are computed using OpenGL Mathematics (GLM) in strict affine order to avoid non-commutative distortion:

$$\mathbf{M}_{\text{model}} = \mathbf{T} \cdot \mathbf{R}_z \cdot \mathbf{R}_y \cdot \mathbf{R}_x \cdot \mathbf{S}$$

1. **Local Scaling ($\mathbf{S}$):** Applied first to transform unit primitives to their physical dimensions without affecting position.
2. **Euler Rotations ($\mathbf{R}_x, \mathbf{R}_y, \mathbf{R}_z$):** Applied around the local origin to orient components correctly.
3. **World Translation ($\mathbf{T}$):** Applied last to place the scaled and rotated object at its target world-space coordinate.

The combined Model-View-Projection (MVP) pipeline transforms local mesh vertices into normalized device coordinates (NDC):

$$\mathbf{v}_{\text{clip}} = \mathbf{M}_{\text{projection}} \cdot \mathbf{M}_{\text{view}} \cdot \mathbf{M}_{\text{model}} \cdot \mathbf{v}_{\text{local}}$$

---

## 4. Texturing Strategy & UV Scale Optimization

Textures were selected to replicate the tactile visual surfaces of the real-world scene:

| Object / Component | Texture Asset | Visual Appearance | UV Scale ($U, V$) | Rationale |
|---|---|---|---|---|
| **Desk Surface** | `rusticwood.jpg` | Natural aged wood grain | $(4.0f, 3.0f)$ | Repeating UV coordinates prevents low-resolution pixelation and eliminates stretching across the large $20 \times 14$ plane. |
| **Coffee Mug Body & Handle** | `drywall.jpg` | White glazed ceramic | $(1.0f, 1.0f)$ | Fine, subtle texture reproduces the smooth, slightly matte finish of glazed ceramic porcelain. |
| **Coffee Liquid Surface** | `breadcrust.jpg` | Dark roasted coffee crema | $(1.0f, 1.0f)$ | Organic color variations and micro-crema textures convincingly simulate brewed coffee. |
| **Eyeglasses Frames & Bridge**| `circular-brushed-gold-texture.jpg`| Brushed gold metallic | $(1.0f, 1.0f)$ | Radial metallic grain produces authentic anisotropic highlights on curved tori. |
| **Eyeglasses Temple Arms** | `knife_handle.jpg` | Dark matte composite metal | $(1.0f, 1.0f)$ | Low-reflectance dark texture provides contrast against the gold rims. |
| **Keyboard Chassis Base** | `stainless.jpg` | Brushed aluminum | $(1.0f, 1.0f)$ | Clean directional grain mimics anodized aluminum hardware. |
| **Keyboard Keybed Block** | `backdrop.jpg` | Segmented keycap grid | $(2.0f, 1.0f)$ | Anisotropic horizontal UV stretch ($2.0\times$) properly aligns matrix rows across the rectangular aspect ratio. |

### Texture Parameters Configured
* **Wrapping Mode:** `GL_REPEAT` for $S$ and $T$ axes to enable seamless tiling.
* **Filtering Mode:** `GL_LINEAR` magnification and `GL_LINEAR_MIPMAP_LINEAR` minification to prevent aliasing and shimmering at grazing viewing angles.
* **Mipmap Generation:** `glGenerateMipmap(GL_TEXTURE_2D)` generated upon load.

---

## 5. Illumination Model & Material Properties

Lighting is calculated per-pixel in the custom GLSL fragment shader using the **Phong Reflection Model**:

$$I_{\text{total}} = I_{\text{ambient}} + I_{\text{diffuse}} + I_{\text{specular}}$$

### 5.1 Light Source Configuration
The scene employs multiple directional and point light sources to establish depth, shape definition, and soft shadows:
* **Key Light:** Primary warm light positioned above and slightly to the right of the desk, providing directional diffuse illumination and primary specular highlights.
* **Fill Light:** Softer, cool ambient light positioned to the left to illuminate shadow regions and reveal side contours.
* **Rim / Accent Light:** Subtle backlight positioned behind the keyboard to separate foreground objects from the dark background.

### 5.2 Phong Equations in GLSL
1. **Ambient Component:**
   $$\mathbf{I}_{\text{ambient}} = \mathbf{L}_{\text{ambient}} + (\mathbf{M}_{\text{ambientColor}} \cdot \mathbf{M}_{\text{ambientStrength}})$$
2. **Diffuse Component:**
   $$\mathbf{I}_{\text{diffuse}} = \max(\mathbf{N} \cdot \mathbf{L}, 0.0) \cdot \mathbf{M}_{\text{diffuseColor}}$$
3. **Specular Component:**
   $$\mathbf{I}_{\text{specular}} = (\mathbf{L}_{\text{specularIntensity}} \cdot \mathbf{M}_{\text{shininess}}) \cdot (\max(\mathbf{V} \cdot \mathbf{R}, 0.0))^{\mathbf{L}_{\text{focalStrength}}} \cdot \mathbf{M}_{\text{specularColor}}$$

---

## 6. Interactive Viewing & Camera System

The application incorporates a flexible virtual camera managed by `ViewManager`:

### 6.1 Camera Controls
* **Translation:** `W` (Forward), `S` (Backward), `A` (Left Strafe), `D` (Right Strafe), `Q` (Elevate Up), `E` (Descend Down).
* **Mouse Look:** Cursor offset calculations adjust camera Yaw and Pitch with smoothed mouse sensitivity.
* **Speed Scaling:** Mouse scroll wheel dynamically modifies camera traversal velocity.
* **Frame-Rate Decoupling:** All movement velocities are scaled by frame delta time ($\Delta t$), ensuring smooth, uniform camera speed across different GPU frame rates.

### 6.2 Dual Projection Modes
* **Perspective Projection (`P` Key):**
  * Configured with an $80^\circ$ Field of View (FOV), aspect ratio matching the display window ($1000/800 = 1.25$), near clipping plane at $0.1f$, and far clipping plane at $100.0f$.
  * Simulates natural human visual perspective with foreshortening.
* **Orthographic Projection (`O` Key):**
  * Configured with a bounding volume scaled to $(-aspect \cdot scale, +aspect \cdot scale, -scale, +scale)$ where $scale = 6.0f$.
  * Removes perspective foreshortening, producing a technical architectural view ideal for inspecting precise parallel alignments across the desk.

---

## 7. Conclusion

By combining modular object-oriented C++ software architecture with mathematically sound transformation pipelines, high-resolution texture mapping, and robust GLSL Phong illumination, the 3D scene successfully replicates the reference scene with high visual fidelity and interactive fluidity.
