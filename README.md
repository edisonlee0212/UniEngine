# UniEngine

UniEngine is an early-stage, cross-platform interactive application and rendering engine for Windows, Linux, and MacOS (Upcoming). 

## Getting Started

The project is a CMake project. For project editing, code inspections, Visual Studio 2017 or 2019 is recommanded. Simply clone/download the project files and open the folder as project in Visual Studio and you are ready.
To directly build the project, scripts under the root folder build.cmd (for Windows) and build.sh (for Linux) are provided for building with single command line.
E.g. For Linux, the command may be :
 - bash build.sh (build in default settings)
 - bash build.sh --clean release (clean and build in release mode)
Please visit script for further details.

## Main features
Here lists the features that already exists in the UniEngine. Some of them I may still developing it.
 - Complete Entity Component System (ECS) 
    - Cache-friendly data component, similiar to the ComponentData in Unity Engine) 
    - Customizable private component, similiar to Component in Unity Engine. 
    - The ECS is intensively used in my other private research projects. It's designed and implemented to imitate the DOTS framework in Unity Engine.
 - Immature Job System for multi-thread support. 
    - The Job System is designed and closely combined with ECS to provide ability to operate concurrently on multiple data components.
 - Fully featured viewer and editor applications
    - Gizmo
    - Scene hierarchy viewing, editing
    - Entity create/delete/rename/set-parent
    - Resource/Asset drag-and-drop
    - GUI registry for Entity Inspector for view/editing for both data component and private component
 - High-fidelity Physically-Based 3D rendering
    - Support for both deferred (RenderSystem) and forward rendering (RenderSystem/native rendering API).
    - Lighting and Shadows
       - Splitting: CSM
       - Warping: CSSM
       - Soft shadow: PCSS
       - Support for dir/point/spot light
    - Post-processing
       - Bloom
       - SSAO (Temporily disabled)
    - PBR with IBL support
       - Environmental map
       - Basic light probe/reflection probe support
    - Instanced rendering
 - 3D Animations
 - Cross-platform support for Linux, Windows
 - Native high-level rendering API support (Please visit RenderManager for further details)
 - Exportable as shared library (For my own research purposes, I'm using the UniEngine as the underlying rendering framework for my other research projects. Those are private.)
 - I/O
    - Asset import
       - Texture (2D, Cubemap)
       - 3D Model
       - 3D animated model (Windows only)
    - Scene import/export (in development stage)
## Upcoming features
Here lists the features that will be introduced to UniEngine in future, though I don't have a concrete plan of when these will come.
- Integrated 3rd party 2D and 3D physics engine
- Procedural terrain and world generation
- Artificial Intelligence
- Audio system
- Documentation
- Event System
## Example projects
- Star Cluster
  - The Star Cluster example shows the potential of Job System with ECS by rendering hundreds of thousands stars at the same time with instanced rendering. The position of each star is calculated in real time in parallel with a single lambda-expression based API similiar to the Entities.ForEach() in Unity. 
- Planet
  - The Planet example shows the ability to use ECS for complex behaviour. The application contains a simple sphere generation program with dynamic LOD calculation based on the position of the scene camera.
- Sample
  - The Sample example is a simple scene with spheres being rendered in different material properties. It's mainly for testing the PBR rendering pipeline.
## Plans
