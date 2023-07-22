#![allow(unused)]
// use crete::entity::Entity;
use crate::{entity::Entity, world::World};
use joker_foundation::{Entry, HashMap};

pub trait MapEntities {
    fn map_entities(&mut self, entity_mapper: &mut EntityMapper);
}

#[derive(Debug, Default)]
pub struct EntityMap {
    map: HashMap<Entity, Entity>,
}

impl EntityMap {
    pub fn insert(&mut self, from: Entity, to: Entity) -> Option<Entity> {
        self.map.insert(from, to)
    }

    pub fn remove(&mut self, entity: Entity) -> Option<Entity> {
        self.map.remove(&entity)
    }

    pub fn entry(&mut self, entity: Entity) -> Entry<'_, Entity, Entity> {
        self.map.entry(entity)
    }

    pub fn get(&self, entity: Entity) -> Option<Entity> {
        self.map.get(&entity).copied()
    }

    pub fn keys(&self) -> impl Iterator<Item = Entity> + '_ {
        self.map.keys().cloned()
    }

    pub fn values(&self) -> impl Iterator<Item = Entity> + '_ {
        self.map.values().cloned()
    }

    pub fn len(&self) -> usize {
        self.map.len()
    }

    pub fn is_empty(&self) -> bool {
        self.map.is_empty()
    }

    pub fn iter(&self) -> impl Iterator<Item = (Entity, Entity)> + '_ {
        self.map.iter().map(|(from, to)| (*from, *to))
    }

    pub fn world_scope<R>(
        &mut self,
        world: &mut World,
        f: impl FnOnce(&mut World, &mut EntityMapper) -> R,
    ) -> R {
        let mut mapper = EntityMapper::new(self, world);
        let result = f(world, &mut mapper);
        mapper.finish(world);
        result
    }
}

pub struct EntityMapper<'m> {
    map: &'m mut EntityMap,
    dead_start: Entity,
    generations: u32,
}

impl<'m> EntityMapper<'m> {
    pub fn get_or_reserve(&mut self, entity: Entity) -> Entity {
        if let Some(mapped) = self.map.get(entity) {
            return mapped;
        }

        let new = Entity {
            generation: self.dead_start.generation + self.generations,
            index: self.dead_start.index,
        };
        self.generations += 1;

        self.map.insert(entity, new);
        new
    }

    pub fn get_map(&'m self) -> &'m EntityMap {
        self.map
    }

    pub fn get_map_mut(&'m mut self) -> &'m mut EntityMap {
        self.map
    }

    fn new(map: &'m mut EntityMap, world: &mut World) -> Self {
        Self {
            map: map,
            dead_start: unsafe { world.entities_mut().alloc() },
            generations: 0,
        }
    }

    fn finish(self, world: &mut World) {
        let entities = unsafe { world.entities_mut() };
        assert!(entities.free(self.dead_start).is_some());
        assert!(entities.reserve_generations(self.dead_start.index, self.generations));
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        entity::{Entity, EntityMap, EntityMapper},
        world::World,
    };

    #[test]
    fn entity_mapper() {
        const FIRST_IDX: u32 = 1;
        const SECOND_IDX: u32 = 2;

        let mut map = EntityMap::default();
        let mut world = World::new();
        let mut mapper = EntityMapper::new(&mut map, &mut world);

        let mapped_ent = Entity::new(FIRST_IDX, 0);
        let dead_ref = mapper.get_or_reserve(mapped_ent);

        assert_eq!(
            dead_ref,
            mapper.get_or_reserve(mapped_ent),
            "should persist the allocated mapping from the previous line"
        );
        assert_eq!(
            mapper.get_or_reserve(Entity::new(SECOND_IDX, 0)).index(),
            dead_ref.index(),
            "should re-use the same index for further dead refs"
        );
        mapper.finish(&mut world);
        // let entity = world.spawn_empty().id();
    }
}
