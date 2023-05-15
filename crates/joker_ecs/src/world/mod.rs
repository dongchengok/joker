#![allow(unused)]

mod identifier;

pub use identifier::WorldId;

use crate::{entity::Entities, component::Components};

pub struct World{
    id:WorldId,
    pub entities:Entities,
    pub components:Components,
    // pub archetypes:Archetypes,
}