#![allow(unused)]
mod map_entities;
pub use map_entities::*;

use serde::{Deserialize, Serialize};

use crate::archetype::{ArchetypeId, ArchetypeRow};
use crate::storage::sparse_set::SparseSetIndex;
use crate::storage::table::{TableId, TableRow};

#[cfg(target_has_atomic = "64")]
use std::sync::atomic::AtomicI64 as AtomicIdCursor;
use std::sync::atomic::Ordering;
use std::{fmt, mem};
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
    //分配过的
    meta: Vec<EntityMeta>,
    //已分配EntityMeta，但是未使用，比如被free了
    pending: Vec<u32>,
    //分配了Entity，但是没有实际分配空间，需要flush，在flush之前，有可能是负数
    free_cursor: AtomicIdCursor,
    //当前使用的
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

    pub fn reserve_entities(&self, count: u32) -> ReserveEntitiesIterator {
        let range_end = self
            .free_cursor
            .fetch_sub(IdCursor::try_from(count).unwrap(), Ordering::Relaxed);
        let range_start = range_end - IdCursor::try_from(count).unwrap();
        //已经分配的可以复用的entity
        let freelist_range = range_start.max(0) as usize..range_end.max(0) as usize;
        let (new_id_start, new_id_end) = if range_start >= 0 {
            //说明没有需要新分配的
            (0, 0)
        } else {
            //重新计算一下需要新分配的
            let base = self.meta.len() as IdCursor;
            let new_id_end = u32::try_from(base - range_start).expect("to many entities");
            let new_id_start = (base - range_end.min(0)) as u32;
            (new_id_start, new_id_end)
        };
        ReserveEntitiesIterator {
            meta: &self.meta[..],
            index_iter: self.pending[freelist_range].iter(),
            index_range: new_id_start..new_id_end,
        }
    }

    pub fn reserve_entity(&self) -> Entity {
        let n = self.free_cursor.fetch_sub(1, Ordering::Relaxed);
        if n > 0 {
            //够用，从池子里预留一个
            let index = self.pending[(n - 1) as usize];
            Entity {
                generation: self.meta[index as usize].generation,
                index,
            }
        } else {
            //不够用，预留一个新的，未初始化
            Entity {
                generation: 0,
                index: u32::try_from(self.meta.len() as IdCursor - n).expect("too many entities"),
            }
        }
    }

    #[inline]
    fn verify_flushed(&mut self) {
        debug_assert!(
            !self.needs_flush(),
            "flush() needs to be called before this operation is legal"
        );
    }

    pub fn alloc(&mut self) -> Entity {
        self.verify_flushed();
        self.len += 1;
        if let Some(index) = self.pending.pop() {
            //有预留的,直接从预留的里面分配一个
            let new_free_cursor = self.pending.len() as IdCursor;
            Entity {
                generation: self.meta[index as usize].generation,
                index: index,
            }
        } else {
            //分配一个新的
            let index = u32::try_from(self.meta.len()).expect("to many entities");
            self.meta.push(EntityMeta::EMPTY);
            Entity {
                generation: 0,
                index: index,
            }
        }
    }

    #[must_use="如果替换了已经存在entity，内存分配的地址会被返回出来，这个必须要处理，否则内存可能泄露"]
    pub fn alloc_at(&mut self, entity: Entity) -> Option<EntityLocation> {
        self.verify_flushed();
        let loc = if entity.index as usize >= self.meta.len() {
            //指定id的entity并不存在，所以要先分配一下
            //meta里直接添加了，就不用flush了，所以使用长度要直接加上去
            self.pending.extend((self.meta.len() as u32)..entity.index);
            let new_free_cursor = self.pending.len() as IdCursor;
            *self.free_cursor.get_mut() = new_free_cursor;
            self.meta
                .resize(entity.index as usize + 1, EntityMeta::EMPTY);
            self.len += 1;
            None
        } else if let Some(index) = self.pending.iter().position(|item| *item == entity.index) {
            //指定id的存在，并且未被使用
            self.pending.swap_remove(index);
            let new_free_cursor = self.pending.len() as IdCursor;
            *self.free_cursor.get_mut() = new_free_cursor;
            self.len += 1;
            None
        } else {
            //已经存在了，会把原来的位置返回给逻辑处理，这里的位置必须有明确处理方式，否则内存就是去管理了
            Some(mem::replace(
                &mut self.meta[entity.index as usize].location,
                EntityMeta::EMPTY.location,
            ))
        };

        self.meta[entity.index as usize].generation = entity.generation;
        loc
    }

    pub fn alloc_at_without_replacement(&mut self, entity: Entity) -> AllocAtWithoutReplacement {
        self.verify_flushed();
        let result = if entity.index as usize >= self.meta.len() {
            self.pending.extend((self.meta.len() as u32)..entity.index);
            let new_free_cursor = self.pending.len() as IdCursor;
            *self.free_cursor.get_mut() = new_free_cursor;
            self.meta
                .resize(entity.index as usize + 1, EntityMeta::EMPTY);
            self.len += 1;
            AllocAtWithoutReplacement::DidNotExist
        } else if let Some(index) = self.pending.iter().position(|item| *item == entity.index) {
            self.pending.swap_remove(index);
            let new_free_cursor = self.pending.len() as IdCursor;
            *self.free_cursor.get_mut() = new_free_cursor;
            self.len += 1;
            AllocAtWithoutReplacement::DidNotExist
        } else {
            let current_meta = &self.meta[entity.index as usize];
            if current_meta.location.archetype_id == ArchetypeId::INVALID {
                AllocAtWithoutReplacement::DidNotExist
            } else if current_meta.generation == entity.generation {
                AllocAtWithoutReplacement::Exists(current_meta.location)
            } else {
                return AllocAtWithoutReplacement::ExistsWithWrongGeneration;
            }
        };

        self.meta[entity.index as usize].generation = entity.generation;
        result
    }

    pub fn free(&mut self, entity: Entity) -> Option<EntityLocation> {
        self.verify_flushed();

        let meta = &mut self.meta[entity.index as usize];
        if meta.generation != entity.generation {
            return None;
        }
        meta.generation += 1;
        let loc = mem::replace(&mut meta.location, EntityMeta::EMPTY.location);
        self.pending.push(entity.index);
        let new_free_cursor = self.pending.len() as IdCursor;
        *self.free_cursor.get_mut() = new_free_cursor;
        self.len -= 1;
        Some(loc)
    }

    pub fn reserve(&mut self, additional: u32) {
        self.verify_flushed();

        let freelist_size = *self.free_cursor.get_mut();
        let shortfall = IdCursor::try_from(additional).unwrap() - freelist_size;
        if shortfall > 0 {
            self.meta.reserve(shortfall as usize);
        }
    }

    pub fn contains(&self, entity: Entity) -> bool {
        self.resolve_from_id(entity.index())
            .map_or(false, |e| e.generation() == entity.generation)
    }

    pub fn clear(&mut self) {
        self.meta.clear();
        self.pending.clear();
        *self.free_cursor.get_mut() = 0;
        self.len = 0;
    }

    #[inline]
    pub fn get(&self, entity: Entity) -> Option<EntityLocation> {
        if let Some(meta) = self.meta.get(entity.index as usize) {
            if meta.generation != entity.generation
                || meta.location.archetype_id == ArchetypeId::INVALID
            {
                return None;
            }
            Some(meta.location)
        } else {
            None
        }
    }

    #[inline]
    pub unsafe fn set(&mut self, index: u32, location: EntityLocation) {
        self.meta.get_unchecked_mut(index as usize).location = location;
    }

    #[must_use = "预留id失败了，必须要处理，可能是逻辑错误"]
    pub fn reserve_generations(&mut self, index: u32, generations: u32) -> bool {
        if (index as usize) >= self.meta.len() {
            return false;
        }
        let meta = &mut self.meta[index as usize];
        if meta.location.archetype_id == ArchetypeId::INVALID {
            meta.generation += generations;
            true
        } else {
            // 说明已经分配了，预留失败了
            false
        }
    }

    pub fn resolve_from_id(&self, index: u32) -> Option<Entity> {
        let idu = index as usize;
        if let Some(&EntityMeta { generation, .. }) = self.meta.get(idu) {
            Some(Entity {
                index: index,
                generation: generation,
            })
        } else {
            let free_cursor = self.free_cursor.load(Ordering::Relaxed);
            let num_pending = usize::try_from(-free_cursor).ok()?;
            (idu < self.meta.len() + num_pending).then_some(Entity {
                generation: 0,
                index,
            })
        }
    }

    //游标后面的部分是分配了，还没初始化的部分
    fn needs_flush(&mut self) -> bool {
        *self.free_cursor.get_mut() != self.pending.len() as IdCursor
    }

    pub unsafe fn flush(&mut self, mut init: impl FnMut(Entity, &mut EntityLocation)) {
        let free_cursor = self.free_cursor.get_mut();
        let current_free_cursor = *free_cursor;

        let new_free_cursor = if current_free_cursor >= 0 {
            current_free_cursor as usize
        } else {
            let old_meta_len = self.meta.len();
            let new_meta_len = old_meta_len + -current_free_cursor as usize;
            self.meta.resize(new_meta_len, EntityMeta::EMPTY);
            self.len += -current_free_cursor as u32;
            for (index, meta) in self.meta.iter_mut().enumerate().skip(old_meta_len) {
                init(
                    Entity {
                        generation: meta.generation,
                        index: index as u32,
                    },
                    &mut meta.location,
                )
            }
            *free_cursor = 0;
            0
        };

        self.len += (self.pending.len() - new_free_cursor) as u32;
        for index in self.pending.drain(new_free_cursor..) {
            let meta = &mut self.meta[index as usize];
            init(
                Entity {
                    index,
                    generation: meta.generation,
                },
                &mut meta.location,
            );
        }
    }

    pub fn flush_as_invalid(&mut self) {
        unsafe {
            self.flush(|_entity, location| {
                location.archetype_id = ArchetypeId::INVALID;
            });
        }
    }

    pub unsafe fn flush_and_reserve_invalid_assuming_no_entities(&mut self, count: usize) {
        let free_cursor = self.free_cursor.get_mut();
        *free_cursor = 0;
        self.meta.reserve(count);
        self.meta.as_mut_ptr().write_bytes(u8::MAX, count);
        self.meta.set_len(count);
        self.len = count as u32;
    }

    //所有使用过的entity
    #[inline]
    pub fn total_count(&self) -> usize {
        self.meta.len()
    }

    //当前使用的entity
    #[inline]
    pub fn len(&self) -> u32 {
        self.len
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.len == 0
    }
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

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::Ordering;

    #[test]
    fn test_automic() {
        let id = std::sync::atomic::AtomicI64::new(0);
        let value = id.fetch_sub(10, Ordering::Relaxed);
        assert_eq!(value, 0);
        let value = id.fetch_sub(10, Ordering::Relaxed);
        assert_eq!(value, -10);
    }

    #[test]
    fn test_range() {
        let mut idx = -1;
        for i in 0..10 {
            idx = i;
        }
        assert_eq!(idx, 9);

        let mut idx = -1;
        for i in 0..=10 {
            idx = i;
        }
        assert_eq!(idx, 10);
    }

    #[test]
    fn test_bits_roundtrip() {
        let e = Entity {
            generation: 0xDEADBEEF,
            index: 0xBAADF00D,
        };
        assert_eq!(Entity::from_bits(e.to_bits()), e);
    }

    #[test]
    fn test_reserve_len() {
        let mut entities = Entities::new();
        entities.reserve_entity();
        unsafe { entities.flush(|_, _| {}) };
        assert_eq!(entities.len(), 1);
    }

    #[test]
    fn test_reserve_and_invalid() {
        let mut entities = Entities::new();
        let e = entities.reserve_entity();
        assert!(entities.contains(e));
        assert!(entities.get(e).is_none());

        unsafe{
            entities.flush(|_,_|{});
        }

        assert!(entities.contains(e));
        assert!(entities.get(e).is_none());
    }

    #[test]
    fn test_const(){
        const C1:Entity = Entity::from_raw(42);
        assert_eq!(42,C1.index);
        assert_eq!(0,C1.generation);

        const C2: Entity = Entity::from_bits(0x0000_00ff_0000_00cc);
        assert_eq!(0x0000_00cc, C2.index);
        assert_eq!(0x0000_00ff, C2.generation);

        const C3: u32 = Entity::from_raw(33).index();
        assert_eq!(33, C3);

        const C4: u32 = Entity::from_bits(0x00dd_00ff_0000_0000).generation();
        assert_eq!(0x00dd_00ff, C4);
    }

    #[test]
    fn test_generation(){
        let mut entities = Entities::new();
        let entity = entities.alloc();
        entities.free(entity);
        assert!(entities.reserve_generations(entity.index, 1));
    }

    #[test]
    fn test_generation_and_alloc(){
        const GENERATIONS:u32 = 10;

        let mut entities = Entities::new();
        let entity = entities.alloc();
        entities.free(entity);

        assert!(entities.reserve_generations(entity.index, GENERATIONS));

        let next_entity = entities.alloc();
        assert_eq!(next_entity.index(), entity.index());
        assert_eq!(next_entity.generation , entity.generation + GENERATIONS + 1);
    }
}
