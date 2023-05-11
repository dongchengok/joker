#![allow(unused)]

use serde::{Deserialize, Serialize};

use crate::archetype::{ArchetypeId, ArchetypeRow};
use crate::storage::table::{TableId, TableRow};

#[derive(Clone, Copy, Deserialize, Serialize, Eq, PartialEq, Ord, PartialOrd, Hash)]
pub struct Entity {
    generation: u32,
    index: u32,
}

pub enum AllocAtWithoutReplacement {
    Exists(EntityLocation),
    DidNotExist,
    ExistsWithWrongGeneration,
}

impl Entity {
    #[cfg(test)]
    pub const fn new(index: u32, generation: u32) -> Entity {
        Entity { index, generation }
    }

    
}

#[derive(Copy, Clone, Debug, PartialEq)]
#[repr(C)]
pub struct EntityLocation {
    pub archetype_id: ArchetypeId,
    pub archetype_row: ArchetypeRow,
    pub table_id: TableId,
    pub table_raw: TableRow,
}

impl EntityLocation {
    const INVALID: EntityLocation = EntityLocation {
        archetype_id: ArchetypeId::INVALID,
        archetype_row: ArchetypeRow::INVALID,
        table_id: TableId::INVALID,
        table_raw: TableRow::INVALID,
    };
}
