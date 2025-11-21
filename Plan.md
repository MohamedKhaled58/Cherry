# Cherry Engine MMORPG Development Plan (2-3 Years)

## Development Philosophy

Following an incremental, game-first approach: build a playable game at each phase, then expand. This ensures we have something working at every milestone rather than building systems in isolation.

---

## Phase 1: Foundation & First Playable (Months 1-6)

**Goal:** Create a simple single-player 3D game with basic character movement

### Month 1-2: 3D Rendering Foundation

- **Perspective Camera System**
  - `Cherry/src/Cherry/Renderer/PerspectiveCamera.h/cpp`
  - `Cherry/src/Cherry/PerspectiveCameraController.h/cpp`
  - FPS-style camera controls (WASD + mouse look)

- **3D Mesh System**
  - `Cherry/src/Cherry/Renderer/Mesh.h/cpp`
  - Basic OBJ file loader
  - Vertex/Index buffer support for 3D data
  - Material system (diffuse color/texture)

- **3D Renderer**
  - `Cherry/src/Cherry/Renderer/Renderer3D.h/cpp`
  - Depth testing and face culling
  - Basic Phong lighting (directional light)
  - Transform system for 3D objects

**Deliverable:** Render a 3D cube/character model with camera controls

### Month 3-4: Basic Gameplay

- **Simple Character Controller**
  - `Cherry/src/Cherry/Gameplay/CharacterController.h/cpp`
  - Ground movement (walk, run)
  - Jump mechanics
  - Basic collision (AABB)

- **Simple World**
  - Flat terrain or simple level
  - Static objects (trees, rocks)
  - Basic scene management

**Deliverable:** Playable single-player demo: walk around, jump, interact with objects

### Month 5-6: Polish & Systems

- **Physics Integration** (Bullet3D)
  - `Cherry/src/Cherry/Physics/PhysicsWorld.h/cpp`
  - Rigid body physics
  - Character controller with physics
  - Collision detection

- **Audio System** (OpenAL or FMOD)
  - `Cherry/src/Cherry/Audio/AudioEngine.h/cpp`
  - Sound effects
  - Background music
  - 3D positional audio

**Deliverable:** Polished single-player demo with physics and audio

---

## Phase 2: Multiplayer Foundation (Months 7-12)

**Goal:** Convert to multiplayer with 2-4 players

### Month 7-8: Network Architecture

- **Client-Server Separation**
  - `Cherry/src/Cherry/Network/Server.h/cpp`
  - `Cherry/src/Cherry/Network/Client.h/cpp`
  - `Cherry/src/Cherry/Network/NetworkManager.h/cpp`
  - TCP for reliable data (chat, inventory)
  - UDP for game state (position, rotation)

- **Protocol Design**
  - `Cherry/src/Cherry/Network/Packet.h`
  - Binary serialization
  - Packet types (Connect, Disconnect, Update, Chat)
  - Compression for large packets

**Deliverable:** Two clients can connect to a server

### Month 9-10: Player Synchronization

- **Entity Replication**
  - `Cherry/src/Cherry/Network/ReplicationManager.h/cpp`
  - Player position/rotation sync
  - Interpolation for smooth movement
  - Lag compensation basics

- **Network Events**
  - `Cherry/src/Cherry/Network/NetworkEvent.h`
  - Event-driven networking
  - Client prediction
  - Server authority validation

**Deliverable:** 2-4 players can move around and see each other

### Month 11-12: Basic Multiplayer Game

- **Simple Combat**
  - Attack animations
  - Damage calculation
  - Health system
  - Death/respawn

- **Chat System**
  - Text chat
  - Command system
  - Chat UI

**Deliverable:** Multiplayer arena: players can fight each other

---

## Phase 3: Core Game Systems (Months 13-18)

**Goal:** Add RPG systems and expand to 10-20 players

### Month 13-14: Character System

- **Character Creation**
  - `Cherry/src/Cherry/Gameplay/Character.h/cpp`
  - Character classes (Warrior, Mage, Archer)
  - Stats system (STR, INT, DEX, etc.)
  - Level system (1-100)
  - Experience points

- **Database Integration**
  - SQLite or MySQL for character data
  - `Cherry/src/Cherry/Database/DatabaseManager.h/cpp`
  - Save/load character data
  - Character persistence

**Deliverable:** Players can create characters, level up, stats persist

### Month 15-16: Inventory & Items

- **Inventory System**
  - `Cherry/src/Cherry/Gameplay/Inventory.h/cpp`
  - `Cherry/src/Cherry/Gameplay/Item.h/cpp`
  - Item database
  - Equipment slots (weapon, armor, accessories)
  - Item stats and bonuses

- **Item System**
  - Item types (weapon, armor, consumable)
  - Item rarity (common, rare, epic, legendary)
  - Item generation

**Deliverable:** Players can pick up items, equip gear, see stat changes

### Month 17-18: Combat & Skills

- **Advanced Combat**
  - `Cherry/src/Cherry/Gameplay/CombatSystem.h/cpp`
  - Skill system (active abilities)
  - Cooldowns
  - Status effects (buff/debuff)
  - Damage types (physical, magical)

- **NPCs & Monsters**
  - `Cherry/src/Cherry/Gameplay/NPC.h/cpp`
  - Basic AI (patrol, chase, attack)
  - Monster spawning
  - Loot drops

**Deliverable:** Players can fight monsters, use skills, get loot

---

## Phase 4: World & Content (Months 19-24)

**Goal:** Expand to full world with quests and 50+ players

### Month 19-20: World System

- **Map/Level System**
  - `Cherry/src/Cherry/World/Map.h/cpp`
  - Multiple maps/zones
  - Map loading/streaming
  - Terrain system
  - Collision meshes per map

- **World Building Tools**
  - Basic level editor
  - Object placement
  - Spawn point management

**Deliverable:** Multiple playable maps/zones

### Month 21-22: Quest System

- **Quest Framework**
  - `Cherry/src/Cherry/Gameplay/Quest.h/cpp`
  - `Cherry/src/Cherry/Gameplay/QuestManager.h/cpp`
  - Quest types (kill, collect, deliver, explore)
  - Quest objectives tracking
  - Quest rewards (XP, gold, items)

- **NPC Dialogue**
  - `Cherry/src/Cherry/Gameplay/DialogueSystem.h/cpp`
  - Dialogue trees
  - Quest givers
  - Shop NPCs

**Deliverable:** Players can accept quests, complete objectives, get rewards

### Month 23-24: Advanced AI & Pathfinding

- **AI System**
  - `Cherry/src/Cherry/AI/AIController.h/cpp`
  - State machine for NPCs
  - Behavior trees
  - A* pathfinding
  - Group AI (monster packs)

- **Economy System**
  - `Cherry/src/Cherry/Gameplay/Economy.h/cpp`
  - Gold/currency system
  - NPC shops
  - Item pricing

**Deliverable:** Rich world with intelligent NPCs, shops, economy

---

## Phase 5: Advanced Features (Months 25-30)

**Goal:** Add advanced systems and support 100+ players

### Month 25-26: Animation System

- **Skeletal Animation**
  - `Cherry/src/Cherry/Renderer/Animation.h/cpp`
  - `Cherry/src/Cherry/Renderer/Animator.h/cpp`
  - Animation loading (glTF/Assimp)
  - Animation blending
  - Animation events

- **Advanced Rendering**
  - Shadow mapping
  - Post-processing effects
  - Particle systems
  - Skybox/environment mapping

**Deliverable:** Animated characters with smooth animations

### Month 27-28: Guild System

- **Guild Framework**
  - `Cherry/src/Cherry/Gameplay/Guild.h/cpp`
  - `Cherry/src/Cherry/Gameplay/GuildManager.h/cpp`
  - Guild creation/management
  - Member roles (leader, officer, member)
  - Guild chat
  - Guild storage

- **PvP System**
  - `Cherry/src/Cherry/Gameplay/PvP.h/cpp`
  - PvP zones
  - PK (Player Killing) system
  - PvP rewards

**Deliverable:** Players can form guilds, PvP combat

### Month 29-30: Trading & Social

- **Trading System**
  - `Cherry/src/Cherry/Gameplay/Trading.h/cpp`
  - Player-to-player trading
  - Trade window
  - Security (anti-scam)

- **Social Features**
  - Friend list
  - Party system (2-8 players)
  - Party bonuses
  - Chat channels (global, guild, party)

**Deliverable:** Full social and trading systems

---

## Phase 6: Polish & Scale (Months 31-36)

**Goal:** Production-ready MMORPG with 200+ concurrent players

### Month 31-32: Performance & Optimization

- **Optimization**
  - Level-of-detail (LOD) system
  - Frustum culling
  - Occlusion culling
  - Network optimization (delta compression)
  - Database query optimization

- **Server Architecture**
  - Multiple server instances
  - Load balancing
  - Server zones/realms
  - Character transfer between servers

**Deliverable:** Optimized engine supporting 200+ players

### Month 33-34: Content Creation Tools

- **Editor Tools**
  - Quest editor
  - NPC editor
  - Item editor
  - Map editor improvements

- **Content Pipeline**
  - Asset management system
  - Hot reloading
  - Version control for game data

**Deliverable:** Tools for content creators

### Month 35-36: Final Polish & Launch Prep

- **UI/UX Polish**
  - Complete UI overhaul
  - Settings menu
  - Keybind customization
  - Tutorial system

- **Testing & Bug Fixes**
  - Comprehensive testing
  - Bug fixing
  - Performance profiling
  - Security audit

- **Documentation**
  - Player guide
  - Developer documentation
  - Server setup guide

**Deliverable:** Production-ready MMORPG

---

## Key Technical Decisions

### Architecture

- **ECS Integration:** Integrate EnTT (already in vendor) for entity management
- **Physics:** Bullet3D for 3D physics (replace Box2D for 3D)
- **Networking:** Custom TCP/UDP hybrid (start simple, optimize later)
- **Database:** SQLite for development, MySQL for production
- **Audio:** OpenAL or FMOD
- **Model Loading:** Assimp library for multiple formats

### File Structure Additions

```
Cherry/src/Cherry/
├── Renderer/
│   ├── PerspectiveCamera.h/cpp
│   ├── Mesh.h/cpp
│   ├── Renderer3D.h/cpp
│   └── Animation.h/cpp
├── Network/
│   ├── Server.h/cpp
│   ├── Client.h/cpp
│   ├── NetworkManager.h/cpp
│   └── Packet.h
├── Gameplay/
│   ├── Character.h/cpp
│   ├── Inventory.h/cpp
│   ├── Item.h/cpp
│   ├── CombatSystem.h/cpp
│   ├── Quest.h/cpp
│   └── Guild.h/cpp
├── Physics/
│   └── PhysicsWorld.h/cpp
├── Audio/
│   └── AudioEngine.h/cpp
├── Database/
│   └── DatabaseManager.h/cpp
├── AI/
│   └── AIController.h/cpp
└── World/
    └── Map.h/cpp
```

### Dependencies to Add

- **Assimp:** 3D model loading
- **Bullet3D:** Physics engine
- **OpenAL/FMOD:** Audio
- **SQLite/MySQL:** Database
- **zlib:** Compression for network packets

---

## Success Metrics by Phase

- **Phase 1:** Single-player 3D game running smoothly
- **Phase 2:** 4 players in same world, synchronized
- **Phase 3:** 20 players, full character progression
- **Phase 4:** 50 players, quest system working
- **Phase 5:** 100 players, all major systems
- **Phase 6:** 200+ players, production-ready

---

## Risk Mitigation

1. **Scope Creep:** Stick to phase goals, defer nice-to-haves
2. **Performance:** Profile early, optimize incrementally
3. **Network Issues:** Start with simple protocol, iterate
4. **Content:** Focus on systems first, content can be added later
5. **Team:** Plan for solo or small team (2-3 people max)

---

## Next Immediate Steps

1. Create PerspectiveCamera system (Week 1)
2. Implement basic 3D mesh loading (Week 2-3)
3. Create Renderer3D (Week 4)
4. Build simple character controller (Week 5-6)
5. Create first playable demo (Week 7-8)

This plan follows an incremental approach: build something playable at each phase, then expand. Each phase delivers a working game that can be tested and enjoyed before moving to the next level of complexity.