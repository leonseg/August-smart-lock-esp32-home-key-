# August Lock Teardown & Motor Identification

Since we are replacing the entire original PCB with a new proto PCB build, the teardown process is simple: open the lock, remove the PCB, and identify the motor and battery connections.

## FCC Reference

- **Model:** ASL-02
- **FCC ID:** 2AB6UASL2
- FCC filings may contain internal photos — check [FCC ID search](https://www.fcc.gov/oet/ea/fccid) for reference images

## Teardown Photos

| | |
|---|---|
| ![August PCB removed](images/august-pcb-removed.jpg) | ![Lock internals with gears](images/lock-internals-gears.jpg) |
| *Original August PCB — component side* | *Lock body with gear train and motor visible* |

> [!TIP]
> Place your own photos in the [`docs/images/`](images/) directory as you go.

## Tools Needed

- Phillips screwdriver (for lock housing screws)
- Small flathead screwdriver or spudger (for flex cable connectors)
- Multimeter (to identify motor wire polarity)
- Camera (to document the teardown)

## Step 1: Open the Lock Housing

1. Remove the 4xAA batteries
2. Remove the back cover screws (4 Phillips screws)
3. Carefully separate the back plate from the body
4. The PCB is visible, connected to the motor and battery connector via flex cables

## Step 2: Remove the Original PCB

1. Disconnect all flex cables from the PCB (battery, motor, LED ring, button)
2. Unscrew the PCB mounting screws (if any)
3. Lift the PCB out of the housing
4. Set it aside — it is no longer needed

> [!NOTE]
> Keep the original PCB in case you want to revert the lock to stock. It's also useful as a reference for connector types.

## Step 3: Identify Motor Wires

The DC motor connects to the lock body with **2 wires** (red and black visible in the photos).

| Wire | Color (typical) | Function |
|------|----------------|----------|
| Motor + | Red | Positive (direction depends on polarity) |
| Motor - | Black | Negative |

To determine lock/unlock direction:
1. Connect the motor wires to a 3V-6V battery temporarily
2. Observe which way the gear train turns
3. Swap polarity and observe the opposite direction
4. Note which polarity = lock (extend deadbolt) and which = unlock (retract)

> [!TIP]
> Mark the wires with tape: "LOCK = Red→+" or whatever your motor's mapping is.

### Motor Characteristics

Measure these values for calibration:

| Parameter | How to Measure | Your Value |
|-----------|---------------|------------|
| Stall current | Apply 6V, hold motor shaft, read amps | _____ mA |
| No-load current | Apply 6V, motor spinning freely | _____ mA |
| Lock rotation time | Time from unlocked to locked at 6V | _____ ms |
| Unlock rotation time | Time from locked to unlocked at 6V | _____ ms |

## Step 4: Identify Battery Connector

The 4xAA battery holder connects via a cable or flex to the PCB area:

| Wire | Voltage | Function |
|------|---------|----------|
| Battery + | ~6V (4×1.5V) | Main power input |
| Battery - | GND | Ground |

Measure with a multimeter to confirm 6V with fresh batteries.

## Step 5: Assess Available Space

The proto PCB and all modules must fit inside the lock housing where the original PCB was.

Measure and record:

| Dimension | Value |
|-----------|-------|
| PCB cavity width | _____ mm |
| PCB cavity height | _____ mm |
| PCB cavity depth (max component height) | _____ mm |
| Mounting hole positions | _____ |

> [!TIP]
> A standard 30×70mm proto PCB should fit in the space where the original PCB sat. Verify with your specific lock before ordering.

## Step 6: Identify What to Keep

**Keep (reuse):**
- DC motor + gear train (in the lock body)
- Lock housing (top and bottom shell)
- 4xAA battery holder
- Thumb-turn mechanism
- Deadbolt adapter / tailpiece

**Remove (replace):**
- Original PCB (STM32L152 + CC2541 + DRV8833 + LIS3DHTR + 74AC541 + voltage regulator)
- Original LED ring PCB (replaced with WS2812B)

## Next Steps

Once the lock is open and the motor/battery connections are identified:

1. [Assemble the new control board](../docs/wiring-guide.md)
2. [Flash the firmware](../docs/flashing-guide.md)
3. [Configure Home Assistant](../home-assistant/configuration.yaml)
