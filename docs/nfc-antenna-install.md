# NFC Antenna Installation Guide

How to install the external NFC antenna in your door so HomeKey can read through the door from the exterior side.

## Why an External Antenna?

The PN532 NFC module will be inside the August lock body on the interior side of the door. NFC has a very short range (~2-4cm). Without an external antenna on the door's exterior, you would need to open the door to tap your phone — defeating the purpose.

The solution: run a small NFC antenna to the exterior side of the door via an IPEX/U.FL cable, flush-mounted in the door edge or face.

## Components

| Component | Specification | Notes |
|-----------|--------------|-------|
| NFC Antenna | 13.56 MHz, IPEX/U.FL connector | ~27×18mm flat patch |
| IPEX cable | 5-10cm, pre-attached to antenna | Connects to PN532 |

## Antenna Placement Options

### Option A: Door Edge (Recommended)

Install the antenna in the edge (chant) of the door, facing the door frame side. This is the most discreet option.

```
        Door (top view)
┌───────────────────────────┐
│                           │
│   Interior     Exterior   │
│                           │
│                           │
│         ┌─────┐           │
│         │Antenna│◄── in edge slot
│         │NFC   │           │
│         └──┬──┘           │
│            │ IPEX cable    │
│            │               │
│     ┌──────▼──────┐       │
│     │   PN532     │       │
│     │   + ESP32   │       │
│     │   (inside   │       │
│     │    August)  │       │
│     └─────────────┘       │
│                           │
└───────────────────────────┘
      ▲
      Door edge where
      antenna is mounted
```

**Pros:** Invisible from both sides, protected from weather.
**Cons:** Requires precise routing in the door edge, user must tap the door edge.

### Option B: Door Face (Exterior)

Mount the antenna flush on the exterior face of the door, near the deadbolt area.

**Pros:** Easier to install, natural tap location.
**Cons:** Visible, may require a decorative cover plate.

### Option C: Behind Door Material

If your door is wood or fiberglass, the antenna may work mounted directly behind the door material (interior side) without any slot.

**Pros:** No modification to door needed.
**Cons:** Reduced range, depends on door material and thickness.

## Installation Steps (Option A — Door Edge)

### Tools Required

- Chisel (15-20mm wide) or router with straight bit
- Drill with 6mm bit (for cable hole)
- Pencil and ruler
- Sandpaper (120 grit)
- Wood filler or small cover plate
- Hot glue or double-sided tape

### Step 1: Mark the Location

1. With the August lock installed, identify where the PN532 module will sit inside the lock body
2. On the door edge, mark a rectangle at the same height:
   - Width: 20mm
   - Height: 30mm
   - Depth: 3-4mm (just enough for the antenna)

```
Door edge view:
┌─────────┐
│         │
│  ┌───┐  │ ◄── 30mm tall
│  │   │  │ ◄── 20mm wide
│  │ A │  │ ◄── 3mm deep
│  │   │  │
│  └───┘  │
│         │
└─────────┘
```

### Step 2: Cut the Slot

1. Use a chisel to carefully carve out the pocket
2. Work slowly — remove 1mm at a time
3. Test-fit the antenna frequently
4. The antenna should sit flush or slightly recessed

> [!TIP]
> If using a router, set the depth stop to 3mm and make multiple passes. A chisel gives more control but takes longer.

### Step 3: Drill the Cable Hole

1. From inside the antenna pocket, drill a 6mm hole angled toward the interior of the door
2. The hole should emerge near where the August lock body sits
3. Deburr the hole on both sides

```
Cross section:
                    ┌──Door──┐
                    │        │
Antenna pocket ──►  │▓▓▓     │
                    │  ╲     │
                    │   ╲    │  ◄── Angled hole for cable
                    │    ╲   │
                    │     ╲──┼──► Cable exits to August body
                    │        │
                    └────────┘
```

### Step 4: Route the Cable

1. Place the antenna in the pocket
2. Feed the IPEX cable through the drilled hole
3. Ensure enough slack for the cable to reach the PN532 without tension
4. Connect the IPEX connector to the PN532's antenna port

### Step 5: Secure and Finish

1. Fix the antenna in the pocket with a thin layer of hot glue or double-sided tape
2. Cover with a thin wood or plastic plate, glued flush
3. Sand and paint/stain to match the door if desired

## Door Material Considerations

> [!WARNING]
> **Metal doors and metal-clad doors will block NFC signals.** Test before cutting!

| Door Material | NFC Signal | Notes |
|--------------|------------|-------|
| Solid wood | Excellent | Works through 10-15mm of wood |
| Hollow wood | Excellent | May need the slot for structural support |
| Fiberglass | Good | Usually transparent to NFC |
| Steel | Blocked | Antenna must be surface-mounted, not behind steel |
| Aluminum-clad wood | Blocked | The aluminum layer blocks NFC — mount antenna where no metal is present |
| Glass | Good | Works through glass panels |

### Testing NFC Transparency

Before cutting into your door:

1. Hold the PN532 module (powered, with antenna attached) on the interior side
2. Try tapping your phone on the exterior side at various positions
3. If you get a read, the material is NFC-transparent at that thickness
4. If no read, you need to surface-mount the antenna

## Read Range Optimization

- **Keep the antenna flat** — bending reduces performance
- **No metal within 10mm** behind the antenna — it acts as a shield
- **Cable length** matters — keep the IPEX cable as short as possible (under 10cm)
- **Antenna orientation** — the flat face should point toward the exterior (toward the phone)
- Typical read range through wood: **2-4cm** (enough for a tap)

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| No NFC read through door | Metal in door, antenna too deep | Test material transparency, reduce depth |
| Intermittent reads | Loose IPEX connector | Reseat connector, add a drop of hot glue to secure |
| Very short range (<1cm) | Cable too long, poor antenna placement | Shorten cable, reposition antenna |
| Works with phone but not Apple Watch | Watch antenna is weaker | Move antenna closer to surface, ensure flush mount |
