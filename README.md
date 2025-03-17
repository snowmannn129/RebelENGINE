# RebelSUITE

A comprehensive CAD/3D modeling software suite with advanced geometry, constraint, and assembly capabilities.

## Project Structure

The project is organized into the following directory structure:

```
RebelSUITE/
├── src/                 # Source code
│   ├── geometry/        # Geometric primitives and operations
│   ├── ui/              # User interface components
│   ├── tools/           # Modeling and editing tools
│   ├── constraints/     # Geometric constraints
│   ├── graphics/        # Rendering and visualization
│   ├── core/            # Core application functionality
│   ├── memory/          # Memory management utilities
│   ├── joints/          # Assembly joint types
│   ├── utils/           # Utility classes and helpers
│   ├── layout/          # UI layout management
│   ├── selection/       # Object selection functionality
│   ├── viewport/        # Viewport and camera management
│   └── scene/           # Scene graph and node management
├── CMakeLists.txt       # Build configuration
└── README.md            # Project documentation
```

## Module Descriptions

### Geometry (`src/geometry/`)
Contains all geometric primitives and related classes including:
- Basic shapes (Circle, Line, Arc, Rectangle, etc.)
- Advanced geometry (NURBS, Bezier curves, Splines, etc.)
- Mesh-related classes
- Bounding geometry

### UI (`src/ui/`)
Contains all user interface components:
- Basic UI widgets (Button, Checkbox, Dropdown, etc.)
- Window management
- Theme and style management
- UI managers and controllers

### Tools (`src/tools/`)
Contains all modeling and editing tools:
- Boolean operations (Union, Subtract, Intersect)
- Surface operations (Extrude, Revolve, Loft, Sweep)
- Edge operations (Fillet, Chamfer, Draft)
- Pattern and mirror tools

### Constraints (`src/constraints/`)
Contains all constraint-related classes:
- Geometric constraints (Angle, Length, Radius, etc.)
- Constraint detection and management

### Graphics (`src/graphics/`)
Contains all rendering and graphics-related code:
- Shaders (OpenGL, Vulkan)
- Rendering systems
- Preview renderers
- Color management

### Core (`src/core/`)
Contains core application functionality:
- Application entry point
- Event system
- File I/O
- Logging
- Transaction management
- Undo/Redo system

### Memory (`src/memory/`)
Contains memory management utilities:
- Allocators
- Smart pointers
- Memory pools
- Garbage collection

### Joints (`src/joints/`)
Contains joint-related classes for assembly:
- Revolute, Prismatic, Cylindrical, and Fixed joints

### Utils (`src/utils/`)
Contains utility classes and helpers:
- Spatial partitioning
- Pattern spacing
- Component positioning
- Boolean operations for subdivision surfaces

### Layout (`src/layout/`)
Contains layout management classes:
- Dock, Flex, Flow, Grid, and Stack layouts
- List views

### Selection (`src/selection/`)
Contains selection-related functionality:
- Selection managers
- Object picking
- Snap management
- Selection visualization

### Viewport (`src/viewport/`)
Contains viewport-related classes:
- Viewport management
- Camera controllers
- View synchronization
- Viewport layouts

### Scene (`src/scene/`)
Contains scene graph and node management:
- Scene nodes
- Scene graph
- Assembly tree
- Renderable nodes

## Build Instructions

To build the project:

1. Make sure you have CMake installed (version 3.15 or higher)
2. Clone the repository
3. Run the following commands:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Development Notes

- The project uses C++17 features
- OpenGL and Vulkan are supported for rendering
- Unit tests are included alongside implementation files
