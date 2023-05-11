#![allow(unused)]

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(transparent)]
pub struct ArchetypeRow(u32);

impl ArchetypeRow {
    pub const INVALID : ArchetypeRow = ArchetypeRow(u32::MAX);

    #[inline]
    pub const fn new(index: usize) -> Self {
        Self(index as u32)
    }

    #[inline]
    pub const fn index(self) -> usize {
        self.0 as usize
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct ArchetypeId(u32);

impl ArchetypeId{
    pub const EMPTY : ArchetypeId = ArchetypeId(0);
    pub const INVALID : ArchetypeId = ArchetypeId(u32::MAX);

    #[inline]
    pub const fn new(index : usize) -> Self
    {
        Self(index as u32)
    }

    #[inline]
    pub const fn index(self) -> usize{
        self.0 as usize
    }
}

#[derive(Copy,Clone)]
pub enum ComponentStatus
{
    Add,
    Mutated,
}

pub struct AddBundle{
    pub archetype_id : ArchetypeId,
    pub bundle_status : Vec<ComponentStatus>,
}

pub trait BundleComponentStatus{
    ///调用者必须保证 index 是合法的
    unsafe fn get_status(&self, index:usize) ->ComponentStatus;
}

impl BundleComponentStatus for AddBundle{
    #[inline]
    unsafe fn get_status(&self, index:usize) ->ComponentStatus {
        *self.bundle_status.get_unchecked(index)
    }
}

pub struct SpawnBundleStatus;

impl BundleComponentStatus for SpawnBundleStatus{
    #[inline]
    unsafe fn get_status(&self, index:usize) ->ComponentStatus {
        ComponentStatus::Add    
    }
}

// #[derive(Default)]
// pub struct Edge{
//     add_bundle: SparesArray<BundleId,AddBundle>,
//     remove_bundle: SparesArray<BundleId, Option<ArchetypeId>>,
// }
