snakes
======

Energy-minimising curves that deform to fit image features

## Options available in snake.json:

### image
- `img_path` – path to image of interest

### snake
- `tension (5)` – elastic snake's internal force
- `stiffness (0.01)` – internal force to resist snake's bending
- `atom (0.1)` – unit of snake's length
- `closed (true)` – should snake be closed or not  
- `implicit (5)` – number of implicitly added joints between sequential explicit two

### algorithm
- `line_weight (-50)` – weight corresponding to region functional
- `edge_weight (0.1)` – weight corresponding to edge functional
- `term_weight (0)` – weight corresponding to termination functional
- `tick (0.0001)` – time between sequential snake's states
- `fixed (false)` – should snake's boundary joints be fixed or not
- `threshold (5)` – value of smallest magnitude on a heat map
