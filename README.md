snakes
======

Energy-minimising curves that deform to fit image features

## Options available in snake.json:

### image
- `img_path` – path to image of interest

### snake
- `tension (10)` – elastic snake's internal force
- `stiffness (0.05)` – internal force to resist snake's bending
- `atom (1)` – unit of snake's length
- `closed (true)` – should snake be closed or not  
- `implicit (10)` – number of implicitly added joints between sequential explicit two

### algorithm
- `line_weight (-15)` – weight corresponding to region functional
- `edge_weight (0)` – weight corresponding to edge functional
- `term_weight (0)` – weight corresponding to termination functional
- `tick (0.0001)` – time between sequential snake's states
- `fixed (false)` – should snake's boundary joints be fixed or not
