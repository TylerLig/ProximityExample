# Unreal Engine Proximity-Based Interaction System

This project extends from the item code defined in https://github.com/TylerLig/StashExample. This project adds functionality for proximity-based interactions. It includes classes and components that manage actors representing items in the game world, with efficient network communication and replication.

## Core Components

### `AProximityItem_Actor`
Represents an item in the game world that can interact with other entities based on proximity. The actor automatically registers itself with the Proximity Game State and initializes from a item definition, creating the necessary item instance and marking it for replication.

### `AProximity_Actor`
Base class that `AProximityItem_Actor` inherits from, providing shared functionality for proximity actors.

### `UProximity_GameStateComponent`
  - Periodically checks and updates the list of proximity actors around each player.
  - Only updates the client if their proximity items have changed.
  - Uses a uniform grid system to find actors within a specified proximity.
