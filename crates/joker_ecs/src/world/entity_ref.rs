#![allow(unused)]

use crate::entity::{Entity, EntityLocation};

use super::World;

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

    #[inline]
    #[must_use = "Omit the .id() call if you do not need to store the `Entity` identifier."]
    pub fn id(&self) -> Entity {
        self.entity
    }

    #[inline]
    pub fn location(&self) -> EntityLocation {
        self.location
    }
}
