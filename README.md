# Game AI - Algorithms

This project implements several AI steering behaviors,  flocking system and space partitioning.

- **Steering Behaviors:** Includes Seek, Flee, Arrive, Face, Pursuit, Evade, and Wander to control agent movement and orientation.
- **Combined Steering & Flocking:** Uses Blended and Priority steering to combine behaviors, and implements Boids flocking with Separation, Cohesion, and Alignment rules for group movement.
- **Optimizations:** Improves performance with **space partitioning** (grid-based neighbor lookup to reduce complexity) and **memory pooling** (fixed-size containers to avoid frequent allocations)