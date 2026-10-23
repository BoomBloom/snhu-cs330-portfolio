# CS 330: Computational Graphics and Visualization — Portfolio Submission

**Student:** Mohannad Abuimeish  
**Course:** CS 330 Computational Graphics and Visualization  
**Institution:** Southern New Hampshire University  

---

## Portfolio Overview

This repository showcases my portfolio artifacts developed throughout **CS 330: Computational Graphics and Visualization**. The portfolio highlights the design, mathematical foundations, low-level implementation, and interactive rendering of a complex 3D virtual environment using **Modern OpenGL**, **C++**, **GLSL (OpenGL Shading Language)**, **GLFW**, **GLEW**, and **GLM (OpenGL Mathematics)**.

### Repository Artifacts
1. **3D Scene Project Archive (`7-1_FinalProjectMilestones_Submission.zip`):** Complete Visual Studio solution, C++ source code, vertex/fragment shaders, procedural 3D meshes, and high-resolution textures comprising the fully realized 3D desktop scene.
2. **Design Decisions Document (`Design_Decisions.md`):** Comprehensive breakdown detailing primitive selection, transformation hierarchies, UV mapping strategies, Phong lighting implementation, and camera projections.
3. **Journal Reflection (`README.md`):** In-depth professional reflection addressing software design methodology, programmatic development strategies, and the academic/professional applications of computational graphics.

---

## 3D Scene Overview & Technical Highlights

The primary project artifact replicates a realistic physical workspace scene deconstructed from a 2D reference image into a multi-object 3D virtual world:

* **Desk Surface:** Scaled planar surface (`PlaneMesh`) with multi-tile UV wrapping ($4 \times 3$ repeat) mapping a natural rustic wood grain texture.
* **Coffee Mug (Composite Object):** 
  * Upright cylindrical ceramic mug body (`CylinderMesh`).
  * Torus-shaped handle (`TorusMesh`) rotated $90^\circ$ around the Z-axis and joined flush to the outer wall.
  * Liquid coffee surface (`CylinderMesh`) nested inside the rim with a roasted coffee crema texture.
* **Eyeglasses (Composite Object):**
  * Dual circular lens frames constructed with thin tori (`TorusMesh`) featuring a circular brushed gold metallic texture.
  * Horizontal cylindrical nose bridge (`CylinderMesh`) joining the frame perimeters.
  * Dual box-extruded temple arms (`BoxMesh`) angled $12^\circ$ outward with a matte dark composite metal finish.
* **Computer Keyboard (Composite Object):**
  * Ergonomically beveled keyboard base shell (`BoxMesh`) with brushed stainless aluminum texturing.
  * Raised keybed cluster block (`BoxMesh`) with an anisotropic $2.0\times$ horizontal UV scale mapping individual key cap matrix patterns.
* **Virtual Camera System:**
  * 6-Degree-of-Freedom (6-DOF) interactive virtual camera supporting translation (`W`, `A`, `S`, `D`, `Q`, `E`), mouse pitch/yaw tracking with sensitivity damping, and scroll-wheel speed scaling.
  * Dual projection modes: perspective projection (`P` key) for realistic human perspective with depth foreshortening, and true orthographic projection (`O` key) for technical CAD/axonometric viewing.
* **Illumination & Material Pipelines:**
  * Custom Phong illumination model implemented in GLSL fragment shaders calculating ambient, diffuse, and specular reflections per fragment across multiple light sources.

---

## Module Eight Journal Reflection

### 1. How Do I Approach Designing Software?

#### What new design skills has your work on the project helped you to craft?
Working on this 3D computational graphics project fundamentally altered how I conceptualize spatial problem-solving and software architecture. Foremost among these skills is **geometric deconstruction**: the ability to analyze complex, organic real-world objects and deconstruct them into elemental geometric primitives (cylinders, tori, boxes, and planes). Rather than viewing an object like an eyeglass frame or coffee mug as an indivisible entity, I learned to dissect it into its constituent dimensions, coordinate frames, and anchor points.

Additionally, I developed deep proficiency with **affine transformation hierarchies**. I learned to structure transformations systematically using linear algebra—strictly sequencing scaling ($S$), rotation ($R_z, R_y, R_x$), and translation ($T$) as matrix products ($M = T \cdot R \cdot S$) to prevent unwanted coordinate distortions. Finally, this project sharpened my architectural separation of concerns: encapsulating GPU resource loading in `ShaderManager`, coordinate/projection transformations in `ViewManager`, procedural geometry in `ShapeMeshes`, and spatial composition in `SceneManager`.

#### What design process did you follow for your project work?
My design process followed a structured, top-down engineering methodology:
1. **Reference Analysis & Primitive Selection:** I began by thoroughly evaluating the 2D reference photograph, identifying the primary objects, analyzing their spatial relationships, and selecting matching 3D geometric primitives.
2. **Mathematical Proportioning & White-Boxing:** Before applying materials or textures, I established a consistent world-space coordinate scale. I placed placeholder wireframe meshes to verify that relative scales, clearances, and alignments (e.g., the coffee mug's height relative to the keyboard base) were physically plausible.
3. **Anchor-Based Hierarchical Grouping:** For composite objects (such as the glasses and coffee mug), I established a single primary anchor position vector in world space and defined all sub-components using relative local offsets. This ensured that repositioning or reorienting an entire composite object required altering only a single root coordinate.
4. **Material, UV Texture, and Shader Pipeline Integration:** Once spatial layout was locked, I designed the texture mapping pipeline, calculating custom UV scale factors to prevent texture stretching over non-uniform aspect ratios.
5. **Lighting and Interactive Feedback Design:** Finally, I implemented the Phong reflection model, balancing ambient coefficients, diffuse light vectors, and specular shininess to achieve realistic material finishes (matte wood, glossy ceramic, brushed metal).

#### How could tactics from your design approach be applied in future work?
The tactics honed during this project translate directly to broader software engineering challenges:
* **Modular Decomposition:** Decomposing complex 3D scenes into primitives mirrors the software engineering practice of breaking monolithic systems into decoupled microservices or modular classes with narrow responsibilities.
* **Layered Abstraction:** By isolating low-level graphics hardware interactions (VBOs, VAOs, GLSL uniform dispatches) behind clean high-level interfaces, the application code remains readable and maintainable. This approach is essential when building complex database engines, networking stacks, or distributed applications.
* **Visual and Incremental Validation:** Verifying each component in isolation prior to integration creates rapid feedback loops that catch logic bugs before they compound into system-wide failures.

---

### 2. How Do I Approach Developing Programs?

#### What new development strategies did you use while working on your 3D scene?
Developing in modern OpenGL required strategies distinct from conventional CPU-bound software development:
* **GPU Pipeline and Memory Management:** I adopted disciplined GPU lifecycle management strategies—generating, binding, configuring, and freeing OpenGL buffers (VAOs, VBOs, EBOs, and texture units) to prevent memory leaks and dangling state machine pointers.
* **Anchor-Relative Procedural Composition:** To avoid hard-coding absolute world coordinates across dozens of shapes, I adopted an anchor-relative development strategy. Each object group defined a base transform matrix, allowing child primitives to inherit transformations predictably.
* **Uniform and Shader State Isolation:** Managing shader uniforms (`bUseTexture`, `bUseLighting`, `objectColor`, `model`, `view`, `projection`) required a defensive state management strategy where uniforms are explicitly validated and set per-draw call to avoid state pollution between objects.

#### How did iteration factor into your development?
Iteration was central to the success of this project:
* **Transformation Tuning:** Initial attempts at placing the eyeglasses' curved temple arms and nose bridge resulted in clipping through the desk surface and improper angles. Iteratively adjusting rotation angles by single-degree increments and testing camera clearances resolved these spatial anomalies.
* **UV Coordinate and Texture Mapping Refinements:** Early texture application resulted in severe stretching on the expansive desk surface and distortion on the keyboard base. Through iterative testing, I introduced dynamic UV scaling parameters, applying a $4.0 \times 3.0$ repeat factor to the rustic wood texture and a $2.0 \times 1.0$ factor to the keyboard keybed matrix to achieve crisp, realistic surface details.
* **Projection and Camera Calibration:** Switching between perspective and orthographic views revealed visual anomalies where orthographic frustum bounds clipped large scene components. Iterating on the orthographic clipping planes and ortho-scale factor ensured smooth, distortion-free transitions across both viewing modes.

#### How has your approach to developing code evolved throughout the milestones, which led you to the project’s completion?
At the outset of the course, my approach to computational graphics was largely procedural and experimental—relying on trial-and-error placement of raw vertices and simple 2D shapes. As milestones progressed, my approach underwent significant maturity:
* **Milestone Two (Primitive Assembly):** Emphasized establishing coordinate frameworks, master-level transform math, and composite shape generation.
* **Milestone Three (Interactive Camera & Navigation):** Required transitioning to responsive real-time event loops, calculating delta time (`gDeltaTime`) to decouple frame rate from camera velocity, and computing view matrix vectors dynamically from mouse input.
* **Milestone Four & Final Project (Lighting, Texturing, & Materials):** Shifted development focus to GPU execution, writing performant GLSL fragment shaders, passing lighting arrays, calculating surface normals, and fine-tuning Phong equations.
* **Module Eight (Physics & Animation):** Extended graphics programming into dynamic kinematic systems, managing velocity vectors, continuous collision detection, and specular reflection physics.

By the completion of the project, my workflow had evolved from writing ad-hoc graphics scripts to architecting a cohesive, reusable, object-oriented rendering framework.

---

### 3. How Can Computer Science Help Me in Reaching My Goals?

#### How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?
Computational graphics and visualization bridge abstract mathematical theory with tangible, empirical software execution. In my continuing academic journey:
* **Mathematical Mastery:** This course transformed linear algebra, vector calculus, and trigonometry—such as dot products for diffuse light angles, cross products for surface normal vectors, and matrix multiplication for coordinate basis changes—from theoretical formulas into intuitive, practical engineering tools.
* **Low-Level Hardware and Parallel Computing:** Understanding GPU pipelines, memory alignment, and shader execution models provides a strong foundation for advanced computer science coursework in parallel computing, high-performance computing (HPC), computer vision, and machine learning visualization.
* **Simulation and Physics Engines:** The kinematic and vector collision models developed in Module 8 establish direct competency for coursework in autonomous systems, robotics simulation, and game engine architecture.

#### How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?
In the professional technology sector, visual computing, simulation, and high-performance systems engineering represent high-demand domains:
* **Direct Industry Applications:** The skills gained in modern C++, OpenGL, and GLSL directly apply to careers in 3D graphics engineering, game engine development (Unreal Engine, Unity), architectural and medical visualization, geographic information systems (GIS), and Computer-Aided Design (CAD) tool development.
* **Virtual, Augmented, and Mixed Reality (VR/AR/XR):** As industries adopt spatial computing, understanding real-time rendering, 6-DOF camera matrices, lighting calculations, and spatial asset optimization is invaluable.
* **Robust Systems Engineering:** Beyond graphics specifically, mastering low-level C++ memory management, pointer manipulation, hardware-accelerated computation, and debugging concurrent GPU/CPU execution makes me a significantly more capable systems programmer, equipped to write reliable, high-performance software across any domain.

---

## Technical Specifications Summary

| Component | Implementation Details |
|---|---|
| **Language & Standard** | C++17, GLSL Version 330 Core |
| **Libraries** | GLFW (Windowing & Input), GLEW (OpenGL Extensions), GLM (Matrix Math), stb_image (Texture Parsing) |
| **Transform Pipeline** | $M_{\text{final}} = T \cdot R_z \cdot R_y \cdot R_x \cdot S$ |
| **Camera Model** | 6-DOF Eulerian camera (Position, Front, Up, Right, Pitch, Yaw), WASD/QE movement, mouse look |
| **Projections** | Perspective ($FOV = 80^\circ$, Near $= 0.1$, Far $= 100.0$) & Orthographic ($Aspect \times Scale$) |
| **Illumination** | Multi-source Phong reflection model: Ambient ($I_a \cdot K_a$) + Diffuse ($I_d \cdot (N \cdot L) \cdot K_d$) + Specular ($I_s \cdot (R \cdot V)^\alpha \cdot K_s$) |
| **Textures Loaded** | Rustic wood (desk), Ceramic porcelain (mug body/handle), Coffee crema (liquid surface), Brushed gold (glasses frame), Matte metal (temple arms), Brushed aluminum (keyboard base), Keybed matrix (keys) |
