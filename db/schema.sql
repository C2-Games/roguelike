-- SQLite schema for game data (enemies, weapons, levels/rooms/map), derived from
-- assets/enemies/, assets/weapons/, and assets/levels/ JSON. See issue #247.

CREATE TABLE enemies (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    class TEXT,
    symbol TEXT
);

CREATE TABLE enemy_tiers (
    id INTEGER PRIMARY KEY,
    enemy_id INTEGER NOT NULL,
    tier INTEGER,
    health INTEGER,
    damage_amount INTEGER,
    damage_type TEXT,
    fov_x INTEGER,
    fov_y INTEGER,
    chase INTEGER,
    speed INTEGER,
    -- Source JSON already has null for enemies with no extra drops.
    extra_drops TEXT,
    FOREIGN KEY (enemy_id) REFERENCES enemies (id)
);

CREATE TABLE weapons (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    type TEXT,
    base_damage INTEGER,
    base_speed INTEGER,
    base_range INTEGER
);

CREATE TABLE weapon_tiers (
    id INTEGER PRIMARY KEY,
    weapon_id INTEGER NOT NULL,
    tier INTEGER,
    damage INTEGER,
    speed INTEGER,
    range INTEGER,
    FOREIGN KEY (weapon_id) REFERENCES weapons (id)
);

CREATE TABLE levels (
    id INTEGER PRIMARY KEY,
    name TEXT,
    description TEXT,
    room_count INTEGER,
    start_room_id INTEGER,
    boss_room_id INTEGER
);

CREATE TABLE rooms (
    id INTEGER PRIMARY KEY,
    level_id INTEGER NOT NULL,
    -- The room's originally-authored id from its source room_N.json, distinct from the
    -- synthetic global id above.
    local_room_id INTEGER,
    name TEXT,
    ref TEXT,
    FOREIGN KEY (level_id) REFERENCES levels (id),
    UNIQUE (level_id, local_room_id)
);

CREATE TABLE room_edges (
    id INTEGER PRIMARY KEY,
    level_id INTEGER NOT NULL,
    from_room_id INTEGER NOT NULL,
    from_door INTEGER,
    to_room_id INTEGER NOT NULL,
    to_door INTEGER,
    FOREIGN KEY (level_id) REFERENCES levels (id),
    FOREIGN KEY (from_room_id) REFERENCES rooms (id),
    FOREIGN KEY (to_room_id) REFERENCES rooms (id)
);

CREATE TABLE room_enemy_spawns (
    id INTEGER PRIMARY KEY,
    room_id INTEGER NOT NULL,
    enemy_name TEXT NOT NULL,
    class TEXT,
    tier INTEGER,
    range_min INTEGER,
    range_max INTEGER,
    FOREIGN KEY (room_id) REFERENCES rooms (id),
    FOREIGN KEY (enemy_name) REFERENCES enemies (name)
);

CREATE TABLE room_loot_spawns (
    id INTEGER PRIMARY KEY,
    room_id INTEGER NOT NULL,
    -- Plain text, not a FK: no loot catalog table exists yet.
    loot_name TEXT,
    class TEXT,
    tier INTEGER,
    range_min INTEGER,
    range_max INTEGER,
    FOREIGN KEY (room_id) REFERENCES rooms (id)
);
