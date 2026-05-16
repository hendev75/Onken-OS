<div align="center">
  <h1>Grapher</h1>
  <p><em>An interactive mathematical expression plotter for BoredOS, supporting both 2D and 3D visualizations.</em></p>
</div>

---

Grapher is a built-in GUI application that lets you type any mathematical equation and see it plotted in real time. It supports 2D explicit and implicit curves as well as full 3D surface visualization — including both explicit surfaces (`z = f(x, y)`) and implicit surfaces (`f(x, y, z) = c`).

> [!NOTE]
> Grapher is located at `src/userland/gui/grapher.c`. It runs as a standard BoredOS GUI process and can be launched from the terminal or from the dock.

---

## Features at a Glance

| Feature | Details |
|---|---|
| **2D Explicit** | Plot `y = f(x)` curves |
| **2D Implicit** | Plot any `f(x, y) = g(x, y)` contour via marching squares |
| **3D Explicit** | Plot `z = f(x, y)` surfaces |
| **3D Implicit** | Plot any `f(x, y, z) = c` surface  |
| **Rendering modes** | Wireframe and filled polygon modes |
| **Height coloring** | Surfaces are colored by a blue→green→yellow→red gradient based on Z height |
| **Phong-style shading** | Filled mode computes per-face normals and applies diffuse + ambient lighting |
| **Parallel rendering** | Evaluation and projection are distributed across 4 worker threads via `sys_parallel_run` |
| **Preset equations** | 7 built-in presets accessible from the toolbar |
| **Auto-fit** | 2D view auto-fits the Y axis to the plotted curve on first plot |
| **Atomic Color-Depth Buffer** | All 3D drawing uses a 64-bit atomic buffer to prevent depth/color race conditions |

---

## Launching Grapher

From the BoredOS terminal:
```sh
grapher
```

Or click the **Grapher icon** in the system dock.

---



### Toolbar Controls

| Control | Function |
|---|---|
| **Equation box** | Type your mathematical expression, then press **Enter** or **Plot** |
| **Plot button** | Parse and render the current equation |
| **Wire / Filled button** | Toggle wireframe vs. shaded polygon mode (3D only) |
| **Presets button** | Open a dropdown of example equations |

### Status Bar Controls (3D mode)

| Control | Function |
|---|---|
| **`+` button** | Increase the 3D world range (zoom out in world space) |
| **`-` button** | Decrease the 3D world range (zoom in in world space) |

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| **Enter** (in equation box) | Plot the equation |
| **Ctrl + R** | Reset the view to defaults |
| **F** | Toggle filled / wireframe rendering (3D mode) |
| **Scroll wheel** | Zoom in/out (2D mode adjusts viewport; 3D mode adjusts camera zoom) |
| **Right-click drag** | Rotate the 3D surface |

---

## Writing Equations

Grapher parses equations entered as plain text. It supports a subset of mathematical notation with automatic implicit multiplication.

### Supported Functions

| Syntax | Meaning |
|---|---|
| `sin(x)` | Sine |
| `cos(x)` | Cosine |
| `tan(x)` | Tangent |
| `sqrt(x)` | Square root |
| `abs(x)` | Absolute value |
| `log(x)` | Natural logarithm (base *e*) |

### Supported Operators

| Operator | Meaning |
|---|---|
| `+` `-` `*` `/` | Arithmetic |
| `^` | Exponentiation (right-associative) |
| `(` `)` | Grouping |

### Special Values

| Token | Value |
|---|---|
| `pi` or `PI` | π ≈ 3.14159… |

### Implicit Multiplication

Adjacent tokens that would normally require a `*` are multiplied automatically:

```
2x        → 2 * x
3sin(x)   → 3 * sin(x)
(x+1)(x)  → (x+1) * x
```

### How Equations Are Classified

Grapher looks at which variables appear in your equation to automatically choose the rendering mode:

| Equation form | Auto-detected as |
|---|---|
| `y = f(x)` or just `f(x)` | 2D explicit |
| `f(x, y) = g(x, y)` | 2D implicit |
| `z = f(x, y)` | 3D explicit |
| `f(x, y, z) = c` | 3D implicit |

If you omit the `=` sign, Grapher treats the input as `y = <expression>` when no `y` or `z` is present, or as `<expression> = 0` otherwise.

---

## Example Equations

### 2D Examples

```
y = sin(x)
y = x^2
y = cos(x)*x
y = abs(x) - 2
x^2 + y^2 = 25          ← circle (implicit)
y = log(x)
```

### 3D Explicit Examples

```
z = sin(x)*cos(y)
z = x^2 - y^2           ← saddle surface
z = sqrt(25 - x^2 - y^2)
```

### 3D Implicit Examples

```
x^2 + y^2 + z^2 = 25    ← sphere
x^2 + y^2 = 16          ← cylinder
x^2 + y^2 - z^2 = 1     ← hyperboloid
```

---

## Navigation Controls

### 2D Mode

| Input | Action |
|---|---|
| **Scroll up** | Zoom in |
| **Scroll down** | Zoom out |
| **Ctrl+R** | Reset to default view (`x: [-10, 10]`) |

### 3D Mode

| Input | Action |
|---|---|
| **Right-click drag** | Rotate the surface (orbit camera) |
| **Scroll up** | Zoom camera in |
| **Scroll down** | Zoom camera out |
| **`+` / `-` buttons** | Increase / decrease world range |
| **Ctrl+R** | Reset rotation and zoom |

> [!TIP]
> In 3D mode, the surface auto-rotates slowly by default. This can be disabled by setting `#define ROTATE 0` in the source file.

---

## Architecture Overview

Grapher is implemented as a single self-contained C file. Below is a high-level breakdown of its major components:

### Math Library

Grapher uses the BoredOS freestanding **`libc/math.h`** library, which provides all the math functions it needs without depending on a host standard library:

| Function | Description |
|---|---|
| `sin`, `cos`, `tan` | Trigonometry via Taylor series (8 terms, range-reduced to `[-π, π]`) |
| `sqrt` | Newton-Raphson iteration (25 steps) |
| `log` | Natural logarithm via Padé-style series |
| `log2`, `log10` | Derived from `log` |
| `exp` | Range-reduced Taylor series for `e^x` |
| `pow` | Integer exponents use fast binary exponentiation; fractional exponents use `exp(e * log(b))` |
| `fabs`, `fmod` | Absolute value and floating-point remainder |
| `floor`, `ceil` | Rounding |
| `sinh`, `cosh`, `tanh` | Hyperbolic functions |
| `hypot`, `fmin`, `fmax`, `fclamp` | Utility helpers |

The constants `M_PI`, `M_E`, `M_LN2`, `M_SQRT2` are also defined in the header.

This library is automatically linked into every userland ELF — any app can `#include "math.h"` to use it.

### Expression Parser

Equations are parsed in three stages:

1. **Tokenizer** (`tokenize`) — converts the input string into a flat token array. Handles implicit multiplication by inserting `*` tokens where needed.
2. **Recursive Descent Parser** (`parse_expr`, `parse_term`, `parse_power`, `parse_unary`, `parse_atom`) — produces an Abstract Syntax Tree (AST) with up to `MAX_NODES = 128` nodes.
3. **Bytecode Compiler** (`compile_ast`) — walks the AST in post-order and emits a flat instruction sequence for a simple stack machine. This avoids recursive evaluation during rendering hot paths.

The resulting bytecode is then executed by `run_bc` for every sample point.

### Rendering Pipeline

#### 2D Rendering

- **Explicit** — evaluates `y = f(x)` at every pixel column and connects adjacent samples with Bresenham lines.
- **Implicit** — applies **marching squares** on a 200×130 grid to find sign changes in `f(x,y) - g(x,y)` and plots intersection pixels.

#### 3D Rendering

The 3D pipeline uses a multi-pass system parallelized across worker threads:

| Pass | Function | Description |
|---|---|---|
| 1 | **Evaluation** | Samples the surface at grid points. For implicit surfaces, this uses **tri-axis marching**. |
| 2 | **Projection** | Projects 3D world coordinates to 2D screen coordinates with perspective. |
| 3 | **Drawing** | Rasterizes wireframe lines or filled triangles with Z-buffering. |

##### Tri-Axis Marching (Implicit Surfaces)

Unlike explicit surfaces that only need one evaluation per grid point, implicit surfaces require finding roots of $f(x, y, z) = 0$. To ensure complete surface connectivity and eliminate "cracks," Grapher marches along all three primary axes:

1.  **X-Axis Pass**: For every $(y, z)$ pair, march along $x$.
2.  **Y-Axis Pass**: For every $(x, z)$ pair, march along $y$.
3.  **Z-Axis Pass**: For every $(x, y)$ pair, march along $z$.

Each pass uses a multi-stage root finder (170 linear steps followed by 15 bisection iterations). By sampling along all three axes, the engine "catches" surfaces that are nearly parallel to any specific marching direction, ensuring that vertical walls and steep gradients are rendered solidly from any viewing angle.

##### Atomic Color-Depth Buffer

To prevent "z-fighting" and race conditions between parallel threads, Grapher uses a 64-bit atomic buffer (`graph_czb`). Each 64-bit word stores:
-   **Upper 32 bits**: Z-depth (integer).
-   **Lower 32 bits**: Pixel color (0xAARRGGBB).

A single `__atomic_compare_exchange_n` operation ensures that a pixel's color and depth are updated together only if the new depth is closer to the camera than the existing one.

Surface normals are estimated using central finite differences of the implicit function.

#### Filled Mod

When filled mode is active, each quad cell is split into two triangles. The average surface normal across the four corner vertices is computed and fed into `apply_shading`, which calculates:

```
intensity = ambient(0.3) + diffuse(0.7) * dot(normal, light_direction)
```

The light direction is fixed at `(0.577, 0.707, 0.408)` (normalized diagonal).

#### Z-Buffer

The depth buffer (`graph_zb`) stores integer depth values. `gfb_pixel_z` uses a **compare-and-swap (CAS) loop** via `__atomic_compare_exchange_n` so multiple parallel draw threads cannot produce race conditions.

### Coordinate Systems

#### 2D

World coordinates map linearly to screen pixels:

```c
screen_x = (wx - view_x_min) / (view_x_max - view_x_min) * graph_w
screen_y = (view_y_max - wy) / (view_y_max - view_y_min) * graph_h
```

#### 3D

Points are first rotated by two Euler angles (`rot_y`, `rot_x`) then projected with a simple perspective divide:

```
persp = d / (pz + d)     // d = range_3d * 5
sx    = px * scale * persp + screen_cx
sy    = -py * scale * persp + screen_cy
```

---

## Configuration Constants

These can be changed at the top of `grapher.c` to tune behaviour:

| Constant | Default | Effect |
|---|---|---|
| `ROTATE` | `1` | Set to `0` to disable auto-rotation in 3D mode |
| `GRID_3D` | `41` | Grid resolution for 3D sampling. Higher = more detail, much slower |

> [!WARNING]
> Setting `GRID_3D` too high (e.g. 9000) will exhaust available memory. The `surf` grid and `surf_x`/`surf_y_3d` arrays are statically allocated at compile time: memory usage grows as **O(GRID_3D²)**. Values above ~512 are not recommended.

> [!TIP]
> `GRID_3D = 256` gives a good balance of detail and performance on typical BoredOS hardware emulation.

---

## Color Palette


3D surfaces are colored by height using a 4-stop rainbow ramp:

```
Low  →  Blue → Cyan → Green → Yellow → Red  →  High
```

---

## Preset Equations

The built-in presets are shown in the dropdown when you click **Presets**:

| Label | Type |
|---|---|
| `y = sin(x)` | 2D explicit |
| `y = x^2` | 2D explicit |
| `y = cos(x)*x` | 2D explicit |
| `z = sin(x)*cos(y)` | 3D explicit |
| `z = x^2 - y^2` | 3D explicit |
| `x^2+y^2+z^2=25` | 3D implicit (sphere) |
| `x^2+y^2=16` | 3D implicit (cylinder) |

---

## Known Limitations

- **No parameter slider** — equations are static; there is no way to animate a parameter.
- **No multiple equations** — only one equation can be graphed at a time.
- **Implicit surface precision** — extremely thin or high-frequency implicit surfaces may still have small artifacts if the grid resolution (`GRID_3D`) is too low.
- **3D implicit performance** — tri-axis marching evaluates the function significantly more times than explicit rendering; high resolutions will impact frame rate.
- **Integer axis labels only for large values** — very large axis values are capped at `>2G` or `<-2G` due to `itoa` limitations.
