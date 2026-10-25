import numpy as np
from PIL import Image

SIZE = 1024
SEED = 7
SUN_DIR = np.array([1.0, 0.08, -1.0])      # where the storm glow sits (match your sunDir direction)
LIGHTNING_DIR = np.array([-0.7, 0.35, -0.6])  # lit-up cloud far off; set LIGHTNING = 0 to disable
LIGHTNING = 1.0

SUN_DIR /= np.linalg.norm(SUN_DIR); LIGHTNING_DIR /= np.linalg.norm(LIGHTNING_DIR)

# palette (display space, 0..1)
ZENITH   = np.array([0.015, 0.025, 0.040])
MID      = np.array([0.060, 0.085, 0.105])
HORIZON  = np.array([0.150, 0.190, 0.210])
GLOW     = np.array([0.650, 0.600, 0.480])
CLOUD_DK = np.array([0.020, 0.030, 0.040])
CLOUD_LT = np.array([0.240, 0.290, 0.320])
SEA      = np.array([0.020, 0.035, 0.045])

def hash2(ix, iy):
    n = (ix * 374761393 + iy * 668265263 + SEED * 1442695041) & 0xFFFFFFFF
    n = ((n ^ (n >> 13)) * 1274126177) & 0xFFFFFFFF
    n = n ^ (n >> 16)
    return (n & 0xFFFFFF) / float(0xFFFFFF)

def vnoise(x, y):
    ix, iy = np.floor(x).astype(np.int64), np.floor(y).astype(np.int64)
    fx, fy = x - ix, y - iy
    fx, fy = fx*fx*(3-2*fx), fy*fy*(3-2*fy)
    a, b = hash2(ix, iy), hash2(ix+1, iy)
    c, d = hash2(ix, iy+1), hash2(ix+1, iy+1)
    return (a*(1-fx)+b*fx)*(1-fy) + (c*(1-fx)+d*fx)*fy

def fbm(x, y, octaves=6):
    v = np.zeros_like(x); amp = 0.5
    for _ in range(octaves):
        v += amp * vnoise(x, y)
        x, y = x*2.03 + 17.1, y*2.03 + 9.7
        amp *= 0.5
    return v / 0.96875

def mix(a, b, t):
    return a + (b - a) * t[..., None]

def sky(d):
    dx, dy, dz = d[..., 0], d[..., 1], d[..., 2]
    up = np.clip(dy, 0, 1)

    base = mix(np.broadcast_to(HORIZON, d.shape), np.broadcast_to(MID, d.shape), np.clip(up*3.0, 0, 1))
    base = mix(base, np.broadcast_to(ZENITH, d.shape), np.clip((up-0.25)*1.6, 0, 1))

    sd = np.clip(np.sum(d * SUN_DIR, axis=-1), 0, 1)
    glow = (0.55*sd**6 + 0.9*sd**40) * np.exp(-np.clip(dy, 0, 1)*5.0)

    # clouds: project onto a plane so they compress toward the horizon
    k = 1.0 / (np.maximum(dy, 0) + 0.10)
    px, pz = dx*k*0.9, dz*k*0.9
    wx = fbm(px*0.6 + 3.0, pz*0.6 + 5.0, 4)
    wz = fbm(px*0.6 + 11.0, pz*0.6 + 1.0, 4)
    big = fbm(px*0.5 + wx*1.8, pz*0.5 + wz*1.8, 6)
    fine = fbm(px*2.2 + wz*1.2, pz*2.2 + wx*1.2, 5)
    cover = np.clip((big - 0.30) / 0.40, 0, 1)
    cover = cover*cover*(3 - 2*cover)

    shade = np.clip(0.25 + 0.9*(fine - 0.5) + 0.8*(big - 0.5), 0, 1)
    lit = mix(np.broadcast_to(CLOUD_DK, d.shape), np.broadcast_to(CLOUD_LT, d.shape), shade)
    lit += GLOW * (glow[..., None] * (0.25 + 0.9*shade[..., None]))      # sun-lit rims

    # lightning: one cloud mass lit from within
    if LIGHTNING > 0:
        ld = np.clip(np.sum(d * LIGHTNING_DIR, axis=-1), 0, 1)
        flash = (ld**30) * (0.4 + 1.0*cover) * LIGHTNING
        lit += np.array([0.55, 0.65, 0.95]) * flash[..., None] * 0.9

    col = mix(base, lit, cover)
    col += GLOW * (glow[..., None] * (1 - cover[..., None]) * 0.6)       # glow through gaps

    # horizon haze so the sea melts into the sky (match uFogColor to this)
    haze = np.exp(-np.maximum(dy, 0)*9.0)
    col = mix(col, np.broadcast_to(HORIZON, d.shape), np.clip(haze*0.85, 0, 1))
    col += GLOW * (glow[..., None] * haze[..., None] * 0.15)

    # below the horizon: dark sea
    below = np.clip(-dy*4.0, 0, 1)
    col = mix(col, np.broadcast_to(SEA, d.shape), below)
    return np.clip(col, 0, 1)

def face_dirs(f, n):
    c = (np.arange(n) + 0.5) / n * 2 - 1
    u, v = np.meshgrid(c, c)           # u: right, v: down  (GL cubemap convention)
    one = np.ones_like(u)
    dx, dy, dz = {
        0: ( one, -v, -u),   # +X right
        1: (-one, -v,  u),   # -X left
        2: ( u,  one,  v),   # +Y top
        3: ( u, -one, -v),   # -Y bottom
        4: ( u, -v,  one),   # +Z front
        5: (-u, -v, -one),   # -Z back
    }[f]
    d = np.stack([dx, dy, dz], axis=-1)
    return d / np.linalg.norm(d, axis=-1, keepdims=True)

names = ["right", "left", "top", "bottom", "front", "back"]
faces = []
for f, name in enumerate(names):
    img = (sky(face_dirs(f, SIZE)) * 255 + 0.5).astype(np.uint8)
    Image.fromarray(img).save(f"{name}.png")
    faces.append(img)
    print("wrote", name)

# preview cross
s = SIZE // 4
cross = Image.new("RGB", (s*4, s*3))
pos = {"top": (1,0), "left": (0,1), "front": (1,1), "right": (2,1), "back": (3,1), "bottom": (1,2)}
for name, img in zip(names, faces):
    cross.paste(Image.fromarray(img).resize((s, s)), (pos[name][0]*s, pos[name][1]*s))
cross.save("preview_cross.png")
print("horizon color for uFogColor:", HORIZON)
