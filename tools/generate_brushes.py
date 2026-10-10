import numpy as np
from PIL import Image
from pathlib import Path

SIZE = 128
OUT = Path(__file__).resolve().parent.parent / "assets" / "brushes"
OUT.mkdir(parents=True, exist_ok=True)
rng = np.random.default_rng(42)

# Normalized coordinates in [-1, 1], pixel centers
ax = (np.arange(SIZE) + 0.5) / SIZE * 2 - 1
X, Y = np.meshgrid(ax, ax)
R = np.sqrt(X**2 + Y**2)

def save(name, mask):
    img = (np.clip(mask, 0, 1) * 255).astype(np.uint8)
    Image.fromarray(img, mode="L").save(OUT / f"{name}.png")
    print("wrote", name)

def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0, 1)
    return t * t * (3 - 2 * t)

# Hard round (slight AA on the edge)
save("round_hard", 1 - smoothstep(0.96, 1.0, R))

# Soft round (gaussian-ish falloff)
save("round_soft", np.exp(-(R * 2.2) ** 2) * (R < 1))

# Medium round (hard core, soft rim)
save("round_medium", 1 - smoothstep(0.5, 1.0, R))

# Square
save("square", ((np.abs(X) <= 0.98) & (np.abs(Y) <= 0.98)).astype(float))

# Flat chisel (rotated ellipse, 45 degrees)
c, s = np.cos(np.pi / 4), np.sin(np.pi / 4)
U, V = X * c + Y * s, -X * s + Y * c
E = np.sqrt((U / 1.0) ** 2 + (V / 0.35) ** 2)
save("chisel", 1 - smoothstep(0.92, 1.0, E))

# Chalk / grain (round shape multiplied by noise)
noise = rng.random((SIZE, SIZE))
noise = (noise > 0.35).astype(float) * (0.6 + 0.4 * rng.random((SIZE, SIZE)))
save("chalk", (1 - smoothstep(0.85, 1.0, R)) * noise)

# Splatter (random dots of varying size inside the circle)
splat = np.zeros((SIZE, SIZE))
for _ in range(60):
    px, py = rng.uniform(-0.85, 0.85, 2)
    if px**2 + py**2 > 0.85**2:
        continue
    r = rng.uniform(0.03, 0.14)
    d = np.sqrt((X - px) ** 2 + (Y - py) ** 2)
    splat = np.maximum(splat, 1 - smoothstep(r * 0.7, r, d))
save("splatter", splat)

# Bristles (vertical streaks inside a round shape)
streaks = np.repeat(rng.random((1, SIZE)), SIZE, axis=0)
streaks = 0.35 + 0.65 * (streaks > 0.4)
save("bristle", (1 - smoothstep(0.9, 1.0, R)) * streaks)