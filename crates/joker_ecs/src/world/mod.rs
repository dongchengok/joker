#![allow(unused)]

mod identifier;

use std::{cell::UnsafeCell, marker::PhantomData, sync::atomic::AtomicU32};

pub use identifier::WorldId;
// pub use entity_ref::{EntityMut};

use crate::{
    archetype::Archetypes,
    component::{Components, Tick},
    entity::{Entities, Entity, EntityLocation, EntityMap},
    storage::Storages,
};

pub struct World {
    id: WorldId,
    pub entities: Entities,
    pub components: Components,
    pub archetypes: Archetypes,
    pub storages: Storages,
    // pub bundles:Bundles,
    // pub removed_components:RemovedComponentEvents,
    // WorldCell使用
    // pub archetype_component_access:ArchetypeComponentAccess,
    pub change_tick: AtomicU32,
    pub last_change_tick: Tick,
    pub last_check_tick: Tick,
}

#[derive(Copy, Clone)]
pub struct EntityRef<'w> {
    world: &'w World,
    entity: Entity,
    location: EntityLocation,
}

pub struct EntityMut<'w> {
    world: &'w mut World,
    entity: Entity,
    location: EntityLocation,
}

impl<'w> EntityMut<'w> {
    #[inline]
    pub(crate) unsafe fn new(
        world: &'w mut World,
        entity: Entity,
        location: EntityLocation,
    ) -> Self {
        debug_assert!(world.entities().contains(entity));
        debug_assert_eq!(world.entities().get(entity), Some(location));

        EntityMut {
            world,
            entity,
            location,
        }
    }
}

#[derive(Copy, Clone)]
pub struct UnsafeWorldCell<'w>(*mut World, PhantomData<(&'w World, &'w UnsafeCell<World>)>);

unsafe impl Send for UnsafeWorldCell<'_> {}
unsafe impl Sync for UnsafeWorldCell<'_> {}

pub trait FromWorld {
    fn from_world(world: &mut World) -> Self;
}

impl<T: Default> FromWorld for T {
    fn from_world(world: &mut World) -> Self {
        T::default()
    }
}

impl<'w> UnsafeWorldCell<'w> {
    #[inline]
    pub(crate) fn new_readonly(world: &'w World) -> Self {
        UnsafeWorldCell(world as *const World as *mut World, PhantomData)
    }

    #[inline]
    pub(crate) fn new_mutable(world: &'w mut World) -> Self {
        Self::new_readonly(world)
    }

    #[inline]
    pub unsafe fn world_mut(self) -> &'w mut World {
        unsafe { &mut *self.0 }
    }

    #[inline]
    pub unsafe fn world(self) -> &'w World {
        unsafe { &*self.0 }
    }

    #[inline]
    pub unsafe fn world_metadata(self) -> &'w World {
        unsafe { self.unsafe_world() }
    }

    #[inline]
    pub unsafe fn unsafe_world(self) -> &'w World {
        unsafe { &*self.0 }
    }
}

impl Default for World {
    fn default() -> Self {
        Self {
            id: WorldId::new().expect("不能创建更多的world了"),
            entities: Entities::new(),
            components: Default::default(),
            archetypes: Archetypes::new(),
            storages: Default::default(),
            // bundles:Default::default(),
            //removed_components:Default::default(),
            // archetype_component_access:Default::default(),
            change_tick: AtomicU32::new(1),
            last_change_tick: Tick::new(0),
            last_check_tick: Tick::new(0),
        }
    }
}

impl World {
    #[inline]
    pub fn new() -> World {
        World::default()
    }

    #[inline]
    pub fn id(&self) -> WorldId {
        self.id
    }

    // #[inline]
    // pub fn as_unsafe_world_cell(&mut self)->UnsafeWorldCell<'_>{
    //     UnsafeWorldCell::new_mutable(self)
    // }

    #[inline]
    pub fn entities(&self) -> &Entities {
        &self.entities
    }

    #[inline]
    pub unsafe fn entities_mut(&mut self) -> &mut Entities {
        &mut self.entities
    }

    pub fn spawn_empty(&mut self) -> EntityMut {
        self.flush();
        let entity = self.entities.alloc();
        unsafe { self.spawn_at_empty_internal(entity) }
    }

    unsafe fn spawn_at_empty_internal(&mut self, entity: Entity) -> EntityMut {
        let archetype = self.archetypes.empty_mut();
        let table_row = self.storages.tables[archetype.table_id()].allocate(entity);
        let location = archetype.allocate(entity, table_row);
        self.entities.set(entity.index(), location);
        EntityMut::new(self, entity, location)
    }

    pub(crate) fn flush(&mut self) {
        // let empty_archetype = self.archetypes
    }
}
