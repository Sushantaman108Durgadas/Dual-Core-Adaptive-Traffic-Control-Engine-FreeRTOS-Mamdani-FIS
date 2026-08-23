# Dual-Core-Adaptive-Traffic-Control-Engine-FreeRTOS-Mamdani-FIS-
"An industrial-grade, multi-threaded traffic automation engine for the ESP32 that uses FreeRTOS to isolate safety-critical signaling from a custom, real-time Mamdani Fuzzy Inference System and a 2-DOF camera tracking gimbal."

## 🚀 Key Features

**Asymmetric Multiprocessing (AMP):** Utilizes FreeRTOS to pin the deterministic traffic signaling sequencer to Core 1, while offloading camera servo mechanics and fuzzy tracking processing to Core 0.

**Custom Mamdani Fuzzy Inference System:** A multi-variable fuzzy engine engineered completely from scratch in C++, featuring triangular membership functions, min-composition mapping rules, and centroid defuzzification to dynamically calculate optimal green phase clearance intervals.

**2-DOF Serial Manipulator Gimbal:** Implements a discrete coordinate transformation matrix using boolean state arrays to achieve complete 270-degree quadrant visual tracking using standard 180-degree positional actuators.

**Thread-Safe Architecture:** Utilizes a low-overhead, synchronized double-buffering scheme at the end of execution rounds to completely eliminate inter-core race conditions and resource contention.

## 🛠️ System Architecture & Data Flow
| ESP32 Core          | Task                            | Function / Flow                             | Output                   |
| ------------------- | ------------------------------- | ------------------------------------------- | ------------------------ |
| **Core 0**          | 📷 Camera Sweep Task            | Captures traffic data from camera           | Camera snapshot          |
| **Core 0**          | 🧠 Mamdani Inference Engine     | Processes traffic density using fuzzy logic | Adaptive junction timing |
| **Core 0 → Core 1** | 🔄 `setJunctionData` Snapshot   | Transfers computed junction data            | Timing parameters        |
| **Core 1**          | ⏱️ Adaptive Timing              | Applies timing parameters deterministically | Updated signal duration  |
| **Core 1**          | 🚦 Deterministic Signaling Task | Controls traffic lights in real time        | Traffic signal states    |

  ## 📈 Fuzzy Engine Logic Matrix

The internal logic maps real-time vehicle density queues against downstream projected traffic flow to evaluate dynamic signal weight distributions:

| Current Intersection Volume | Upstream Projected Inflow | Resolved Signal Phase Weight |
| :-------------------------: | :-----------------------: | :--------------------------: |
|       Equal ($i = j$)       |      Equal ($i = j$)      |      **Medium Phase**        |
|       Low ($i < j$)         |      High ($i < j$)       |      **High Phase**          |
|       High ($i > j$)        |      Low ($i > j$)        |      **Low Phase**           |

## 📦 Repository Structure

* `sketch.ino`: Main firmware core managing task structures, hardware maps, and intersection sequencing.
* `diagram.json`: Digital schematic blueprint for Wokwi simulation testing environments.
