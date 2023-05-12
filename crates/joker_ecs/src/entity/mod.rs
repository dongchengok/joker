#![allow(unused)]

use serde::{Deserialize, Serialize};

use crate::archetype::{ArchetypeId, ArchetypeRow};
use crate::storage::sparse_set::SparseSetIndex;
use crate::storage::table::{TableId, TableRow};

use std::fmt;
#[cfg(target_has_atomic = "64")]
use std::sync::atomic::AtomicI64 as AtomicIdCursor;
#[cfg(target_has_atomic = "64")]
type IdCursor = i64;

// 有些系统不支持64位的情况下用下面这个
#[cfg(not(target_has_atomic = "64"))]
use std::sync::atomic::AtomicIsize as AtomicIdCursor;
#[cfg(not(target_has_atomic = "64"))]
type IdCursor = isize;

#[derive(Clone, Copy, Eq, PartialEq, Ord, PartialOrd, Hash)]
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

    pub const PLACEHOLDER: Self = Self::from_raw(u32::MAX);

    pub const fn from_raw(index: u32) -> Entity {
        Entity {
            generation: 0,
            index: index,
        }
    }

    pub const fn to_bits(self) -> u64 {
        (self.generation as u64) << 32 | self.index as u64
    }

    pub const fn from_bits(bits: u64) -> Self {
        Self {
            generation: (bits >> 32) as u32,
            index: bits as u32,
        }
    }

    #[inline]
    pub const fn index(self) -> u32 {
        self.index
    }

    #[inline]
    pub const fn generation(self) -> u32 {
        self.generation
    }
}

impl Serialize for Entity {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        serializer.serialize_u64(self.to_bits())
    }
}

impl<'de> Deserialize<'de> for Entity {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let id: u64 = serde::de::Deserialize::deserialize(deserializer)?;
        Ok(Entity::from_bits(id))
    }
}

impl fmt::Debug for Entity {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}v{}", self.index, self.generation)
    }
}

impl SparseSetIndex for Entity {
    #[inline]
    fn sparse_set_index(&self) -> usize {
        self.index() as usize
    }

    #[inline]
    fn get_sparse_set_index(value: usize) -> Self {
        Entity::from_raw(value as u32)
    }
}

pub struct ReserveEntitiesIterator<'a> {
    meta: &'a [EntityMeta],
    index_iter: std::slice::Iter<'a, u32>,
    index_range: std::ops::Range<u32>,
}

impl<'a> Iterator for ReserveEntitiesIterator<'a> {
    type Item = Entity;

    fn next(&mut self) -> Option<Self::Item> {
        self.index_iter
            .next()
            .map(|&index| Entity {
                generation: self.meta[index as usize].generation,
                index: index,
            })
            .or_else(|| {
                self.index_range.next().map(|index| Entity {
                    generation: 0,
                    index: index,
                })
            })
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = self.index_iter.len() + self.index_range.len();
        (len, Some(len))
    }
}

impl<'a> std::iter::ExactSizeIterator for ReserveEntitiesIterator<'a> {}
impl<'a> std::iter::FusedIterator for ReserveEntitiesIterator<'a> {}

#[derive(Debug)]
pub struct Entities {
    meta: Vec<EntityMeta>,
    pending: Vec<u32>,
    free_cursor: AtomicIdCursor,
    len: u32,
}

impl Entities {
    pub const fn new() -> Self {
        Entities {
            meta: Vec::new(),
            pending: Vec::new(),
            free_cursor: AtomicIdCursor::new(0),
            len: 0,
        }
    }

    // pub fn reserve_entities(&self, count:u32)->ReserveEntitiesIterator{
    //     let range_end = self.free_cursor.fetch_sub(IdCursor::try_from(count).unwrap(), Ordering::Relaxed)
    // }
}

#[derive(Clone, Copy, Debug)]
#[repr(C)]
struct EntityMeta {
    pub generation: u32,
    pub location: EntityLocation,
}

impl EntityMeta {
    const EMPTY: EntityMeta = EntityMeta {
        generation: 0,
        location: EntityLocation::INVALID,
    };
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
