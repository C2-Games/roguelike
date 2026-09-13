-- Seed data for game_data.db, dumped from the JSON-derived database
-- built by issue #247. See issue #248: this replaces JSON as the
-- checked-in source of truth -- generate_db now just execs schema.sql
-- then this file, no JSON parsing involved.

INSERT INTO enemies (id, name, class, symbol) VALUES (1, 'goblin', 'base', 'G');

INSERT INTO weapons (id, name, type, base_damage, base_speed, base_range) VALUES (1, 'bow', 'Ranged', 10, 5, 15);
INSERT INTO weapons (id, name, type, base_damage, base_speed, base_range) VALUES (2, 'fisticuff', 'Melee', 5, 10, 1);

INSERT INTO levels (id, name, description, room_count, start_room_id, boss_room_id) VALUES (1, 'Level 1', 'Goblin''s Den', 8, 1, 8);

INSERT INTO enemy_tiers (id, enemy_id, tier, health, damage_amount, damage_type, fov_x, fov_y, chase, speed, extra_drops) VALUES (1, 1, 1, 50, 10, 'base', 20, 10, 5, 10, NULL);
INSERT INTO enemy_tiers (id, enemy_id, tier, health, damage_amount, damage_type, fov_x, fov_y, chase, speed, extra_drops) VALUES (2, 1, 2, 100, 10, 'poison', 20, 10, 5, 10, NULL);

INSERT INTO weapon_tiers (id, weapon_id, tier, damage, speed, range) VALUES (1, 1, 1, 15, 7, 20);
INSERT INTO weapon_tiers (id, weapon_id, tier, damage, speed, range) VALUES (2, 1, 2, 20, 9, 25);
INSERT INTO weapon_tiers (id, weapon_id, tier, damage, speed, range) VALUES (3, 2, 1, 7, 12, 1);
INSERT INTO weapon_tiers (id, weapon_id, tier, damage, speed, range) VALUES (4, 2, 2, 10, 15, 1);

INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (1, 1, 1, 'Entrance Hall', 'cross_hall.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (2, 1, 2, 'Goblin Lair', 'rect_pillar_hall.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (3, 1, 3, 'Chamber of Shadows', 'chambered.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (4, 1, 4, 'Twin Halls', 'twin_halls.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (5, 1, 5, 'Hall of Echoes', 'l_shape.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (6, 1, 6, 'Maze of Whispers', 'maze.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (7, 1, 7, 'Ruins of the Goblin King', 'ruins.txt');
INSERT INTO rooms (id, level_id, local_room_id, name, ref) VALUES (8, 1, 8, 'Goblin King''s Lair', 'rect_plain.txt');

INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (1, 1, 1, 1, 2, 3);
INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (2, 1, 1, 2, 3, 4);
INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (3, 1, 1, 4, 4, 2);
INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (4, 1, 2, 2, 6, 4);
INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (5, 1, 3, 1, 7, 3);
INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (6, 1, 5, 1, 6, 3);
INSERT INTO room_edges (id, level_id, from_room_id, from_door, to_room_id, to_door) VALUES (7, 1, 5, 4, 8, 1);

INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (1, 1, 'goblin', 'base', 1, 2, 4);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (2, 2, 'goblin', 'base', 1, 3, 7);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (3, 2, 'goblin', 'base', 2, 0, 3);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (4, 3, 'goblin', 'base', 2, 2, 8);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (5, 4, 'goblin', 'base', 1, 2, 4);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (6, 5, 'goblin', 'base', 1, 3, 6);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (7, 5, 'goblin', 'base', 2, 0, 3);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (8, 6, 'goblin', 'base', 1, 2, 5);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (9, 6, 'goblin', 'base', 2, 1, 2);
INSERT INTO room_enemy_spawns (id, room_id, enemy_name, class, tier, range_min, range_max) VALUES (10, 7, 'goblin', 'base', 1, 1, 3);

INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (1, 1, 'gold_pouch', 'currency', 1, 1, 3);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (2, 2, 'gold_pouch', 'currency', 1, 4, 8);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (3, 2, 'health_pack', 'health', 1, 1, 2);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (4, 3, 'health_pack', 'health', 2, 1, 2);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (5, 4, 'gold_pouch', 'currency', 1, 2, 5);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (6, 5, 'gold_pouch', 'currency', 1, 3, 6);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (7, 5, 'health_pack', 'health', 2, 1, 2);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (8, 6, 'gold_pouch', 'currency', 1, 3, 6);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (9, 6, 'weapon_upgrade', 'weapon', 2, 1, 1);
INSERT INTO room_loot_spawns (id, room_id, loot_name, class, tier, range_min, range_max) VALUES (10, 7, 'gold_pouch', 'currency', 1, 1, 3);

