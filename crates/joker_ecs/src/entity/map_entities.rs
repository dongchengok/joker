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

pub struct EntityMapper<'m> {
    map: &'m mut EntityMap,
    dead_start: Entity,
    generations: u32,
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

    // pub fn world_scope<R>(&mut self, world:&mut World, f:impl FnOnce(&mut World, &mut EntityMapper)->R)->R{
    //     let mut mapper = EntityMapper::new(self,world);
    //     let result = f(world,&mut mapper);
    //     mapper.finish(world);
    //     result
    // }
}

impl<'m> EntityMapper<'m> {
    pub fn get_or_reserve(&mut self, entity: Entity) -> Entity {
        if let Some(mapped) = self.map.get(entity) {
            return mapped;
        }

        todo!("没看懂这块！");
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

    // fn new(map: &'m mut EntityMap, world: &*mut World) -> Self {
    //     Self {
    //         map: map,
    //         dead_start: unsafe { world.entities_mut().alloc },
    //         generations: 0,
    //     }
    // }
}
