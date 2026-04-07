# Physics Tuning Sheet

## Purpose

This document turns the early flight-physics target into a small, testable tuning scope.

The goal is not full CFD. The goal is a believable paraglider feel that:

- reads clearly to the player
- behaves consistently enough to tune
- stays portable across renderer or engine changes

## V1 Simulation Boundary

The first-pass model should own:

- pilot control state
- wing energy state
- air-mass effects
- terrain-relative flight phase
- landing outcome

It should not try to own:

- full cloth canopy simulation
- volumetric CFD
- weather forecasting
- highly local rotor detail beyond a tuned gameplay approximation

## Pilot Inputs

Expose these as explicit control channels:

- `leftBrake`
- `rightBrake`
- `symmetricBrake`
- `weightShift`
- `speedBar`
- `flareIntent`

Expected effects:

- left and right brake change roll, yaw tendency, and sink
- symmetric brake trades speed for slower forward travel and more sink, until flare window or stall risk
- weight shift adds lower-authority roll input with less drag than brake
- speed bar raises speed and glide penetration while increasing sink and reducing flare margin
- flare intent should be modeled as timing-sensitive energy conversion near ground, not as a generic lift button

## Wing State

Track and tune these core state variables:

- `airspeedKmh`
- `groundSpeedKmh`
- `verticalSpeedMps`
- `headingDeg`
- `bankDeg`
- `pitchDeg`
- `turnRateDegPerSecond`
- `groundClearanceMeters`
- `stallWarning`
- `flareEffectiveness`
- `flightPhase`

Derived values can grow later, but V1 should stay readable.

## Air-Mass Stack

Treat the atmosphere as layered contributions that sum into the current frame:

### Baseline

- trim glide sink
- speed-dependent sink penalty
- turn-induced sink
- brake-induced sink

### Wind

- prevailing wind direction and speed
- advection of the wing through the air mass
- optional altitude-based wind gradient later

### Ridge Lift

- strongest on windward faces
- fades with distance from the ridge band
- increases with better wind alignment

### Lee Sink and Rotor Proxy

- sink behind terrain
- optional turbulence amplification in the lee
- no attempt at full recirculating flow in V1

### Thermals

- drifting cores
- stronger lift in the core, softer lift near the edge
- sink ring outside the thermal

### Turbulence

- low-amplitude vertical gusts
- roll disturbance
- damping from assist or stability systems if needed

## Five Tuning Scenarios

These scenarios should drive nearly all early tuning work.

### 1. Ridge Pass

Setup:

- medium wind, aligned with ridge
- wing enters windward lift band at moderate speed

What should happen:

- lift should build progressively, not snap
- strongest climb should happen where terrain and wind alignment make sense
- moving too far back should reduce lift or enter sink

Key outputs:

- ridge lift contribution
- net vario
- turn authority near terrain

### 2. Thermal Climb

Setup:

- light-to-medium wind
- one drifting thermal with clear core and edge

What should happen:

- entering the edge should feel noticeable but not binary
- centering should improve climb
- drifting off-core should degrade climb without instantly killing it

Key outputs:

- thermal lift contribution
- thermal sink-ring penalty
- climb stability during gentle circles

### 3. Glide Transition

Setup:

- neutral air with some wind
- pilot alternates trim, brake, and speed bar states

What should happen:

- speed bar should increase penetration and lower climb performance
- deeper brake should slow the wing and worsen sink
- over-brake should push toward stall warning rather than act like a free slow-flight mode

Key outputs:

- airspeed range
- glide ratio trend
- stall-warning behavior

### 4. Approach

Setup:

- low altitude
- target landing zone
- mild crosswind or headwind variations

What should happen:

- approach should reward stable alignment and moderate energy control
- excessive bank or brake near ground should feel risky
- the player should be able to read whether they are high, low, fast, or unstable

Key outputs:

- groundspeed
- vertical speed
- approach error
- phase transitions into `approach`

### 5. Flare

Setup:

- final meters before touchdown
- repeat with early, correct, late, and over-strong flare timing

What should happen:

- correct flare should reduce touchdown energy
- early flare should bleed speed too soon and punish the landing
- late flare should not save a hard arrival
- over-flare should carry stall risk

Key outputs:

- flare effectiveness
- touchdown vertical speed
- touchdown groundspeed
- landing rating

## First-Pass Parameter Table

These parameters should be externalized or at least grouped clearly in code.

### Baseline Wing

- trim airspeed
- min controllable airspeed
- max airspeed
- baseline sink rate
- speed-to-sink curve

### Controls

- brake-to-drag multiplier
- brake-to-roll multiplier
- weight-shift authority
- speed-bar acceleration effect
- input response smoothing

### Stability

- stall onset threshold
- stall sink multiplier
- gust roll magnitude
- turbulence damping

### Lift Sources

- ridge peak lift
- ridge depth falloff
- lee sink strength
- thermal core radius
- thermal edge falloff
- thermal sink ring strength
- thermal drift factor

### Landing

- flare window height
- flare lift multiplier
- flare drag multiplier
- smooth landing threshold
- hard landing threshold
- crash threshold

## Telemetry To Show While Tuning

The debug HUD should expose:

- current phase
- airspeed
- groundspeed
- vertical speed
- bank
- pitch
- ground clearance
- stall warning
- flare effectiveness
- ridge lift contribution
- thermal lift contribution
- sink contribution
- landing zone distance and approach error

Without this, tuning gets subjective too quickly.

## Acceptance Criteria

V1 is good enough to move forward when:

- a held brake input produces a progressive turn, not a snap roll
- ridge lift can be found and used intentionally
- thermal centering changes climb quality in a readable way
- speed bar, trim, and brake each produce distinct glide tradeoffs
- approach and flare timing matter
- the same test setup produces repeatable results frame to frame

## Research Mapping

Use references narrowly:

- `rayleigh_benard` for convection and buoyancy intuition
- `shear_flow` for shear-layer and turbulence intuition
- `planetswe` for large-scale flow intuition

Use practical paragliding references for:

- glide polar estimates
- flare timing
- landing technique
- ridge and thermal pilot heuristics

`the_well` should inform intuition and shaping of fields, not replace the gameplay simulation with a heavyweight solver.
