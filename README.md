# Uberknall Inspired — VST3 source project (v1)

A small JUCE/CMake project for a first-pass Uberknall-inspired guitar plugin. It has two independent bypasses: Drive and Tone Stack. Controls are Gain, Bass, Mid, Treble, Presence, and Master.

## Important modeling note

This is a **DSP approximation**, not an electrically exact simulation of the Bogner pedal or the PCB Guitar Mania clone. The drive section currently uses cascaded asymmetric soft-clipping; the tone section uses simple bass shelf, mid bell, treble shelf, and presence bell filters. The current center frequencies/Q values are starting voicing choices, not values derived from a verified schematic. The supplied audio is a listening reference, not enough by itself to identify the circuit transfer functions.

The earlier Uberknall + Amp Tone Stack build document is the better circuit reference for a later topology-derived revision. Before claiming circuit accuracy, verify the complete schematic wiring, pot tapers/lug connections, component values, loading, active-stage gain, and Presence feedback network.

## Build on GitHub (Windows)

1. Create a new GitHub repository.
2. Upload the contents of this ZIP to the repository root (not the containing folder).
3. Commit to the `main` branch.
4. Open **Actions**. The workflow named **Build Windows VST3** should run automatically; you can also run it manually with **Run workflow**.
5. When it finishes, open the workflow run and download the artifact named `Uberknall-Inspired-Windows-VST3`.
6. Extract the artifact. The `.vst3` bundle can be installed in your DAW's VST3 folder.

GitHub Actions downloads JUCE 8.0.6 during configuration and builds the plugin. You need not upload JUCE yourself. If GitHub reports a build error, share the log and we can fix it.

## Local build (optional)

Requires CMake, a C++17 compiler, and internet access for JUCE FetchContent:

```bash
cmake -S . -B build
cmake --build build --config Release --target UberknallInspired_VST3
```

## Scope of v1

- Drive stage with independent bypass
- Tone stack with independent bypass
- Gain, Bass, Mid, Treble, Presence, Master
- Mono/stereo support
- Parameter state save/restore
- No extra modules, routing matrix, presets browser, or additional effects

## Next modeling pass

Replace the generic DSP voicing with circuit-derived models, one section at a time. Keep Drive and Tone Stack separate so each can be compared and refined independently against the reference pedal/recordings.
