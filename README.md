# Mahjong 3DS (`3ds_mahjong`)

<p align="center">
  <img src="docs/icon.png" width="72" height="72" alt="3ds_mahjong icon" />
  &nbsp;&nbsp;&nbsp;
  <img src="docs/banner.png" width="256" height="128" alt="3ds_mahjong 3DS banner" />
</p>

<p align="center">
  <img src="docs/screenshot.png" width="360" alt="3ds_mahjong dual-screen 3DS preview" />
</p>

> **3D Pyramid Tile Solitaire (`CTR-P-MAHJ`, UniqueId `0x0D3EF`)** — Part of the [**lafiabrew**](https://github.com/orgs/lafiabrew/repositories) Nintendo 3DS Homebrew Suite.

3D multi-layer Mahjong Solitaire (Bamboo, Dots, Winds, Dragon tiles) with upper-layer occlusion, free-edge validation, and auto-hint matcher.

---

## Architecture & Data Flow

```mermaid
flowchart LR
    PYR[36-Tile 2-Layer Pyramid] --> FREE[Free-Tile Occlusion & Edge Check]
    FREE -->|A Match Pair / X Hint| SCORE[Pair Removal & Score]
```

---

## Controls & Touch Ergonomics

| Button / Input | Action |
|---|---|
| `D-Pad` | Move cursor across the 36-tile pyramid |
| `A` | Select and match two free tiles with identical faces |
| `X` | Auto-find and match a valid free pair (Hint) |
| `Y` | Shuffle and deal a new pyramid |

---

## Build, Verify & Install

```bash
make test
make 3ds && make verify-3ds
make qr
```
