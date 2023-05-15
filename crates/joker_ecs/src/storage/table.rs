#![allow(unused)]

use joker_ptr::OwningPtr;

use super::blob_vec::BlobVec;
use crate::component::{ComponentInfo, Tick};
use std::{cell::UnsafeCell, alloc::Layout};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableId(u32);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableRow(u32);

#[derive(Debug)]
pub struct Column {
    data: BlobVec,
    added_ticks: Vec<UnsafeCell<Tick>>,
    changed_ticks: Vec<UnsafeCell<Tick>>,
}

impl TableId {
    pub const INVALID: TableId = TableId(u32::MAX);

    #[inline]
    pub fn new(index: usize) -> Self {
        TableId(index as u32)
    }

    #[inline]
    pub fn index(&self) -> usize {
        self.0 as usize
    }

    #[inline]
    pub fn empty() -> Self {
        TableId(0)
    }
}

impl TableRow {
    pub const INVALID: TableRow = TableRow(u32::MAX);

    #[inline]
    pub fn new(index: usize) -> Self {
        Self(index as u32)
    }

    #[inline]
    pub fn index(&self) -> usize {
        self.0 as usize
    }
}

impl Column {
    #[inline]
    pub fn with_capacity(component_info: &ComponentInfo, capacity: usize) -> Self {
        Self {
            data: unsafe { BlobVec::new(component_info.layout(), component_info.drop(), capacity) },
            added_ticks: Vec::with_capacity(capacity),
            changed_ticks: Vec::with_capacity(capacity),
        }
    }

    #[inline]
    pub fn item_layout(&self)->Layout{
        self.data.layout()
    }

    #[inline]
    pub unsafe fn initialize(&mut self, row: TableRow, data:OwningPtr<'_>, tick:Tick){
        debug_assert!(row.index()<self.len());
        self.data.initialize_unchecked(row.index(), data);
        *self.added_ticks.get_unchecked_mut(row.index()).get_mut() = tick;
        *self.changed_ticks.get_unchecked_mut(row.index()).get_mut() = tick;
    }

    #[inline]
    pub unsafe fn replace(&mut self, row:TableRow, data:OwningPtr<'_>, change_tick:Tick){
        debug_assert!(row.index()<self.len());
        self.data.replace_unchecked(row.index(), data);
        *self.changed_ticks.get_unchecked_mut(row.index()).get_mut() = change_tick;
    }

    #[inline]
    pub unsafe fn replace_untracked(&mut self, row:TableRow, data:OwningPtr<'_>){
        debug_assert!(row.index()<self.len());
        self.data.replace_unchecked(row.index(), data);
    }

    #[inline]
    pub fn len(&self)->usize{
        self.data.len()
    }

    #[inline]
    pub fn is_empty(&self)->bool{
        self.data.is_empty()
    }

    #[inline]
    pub unsafe fn swap_remove_unchecked(&mut self, row:TableRow){
        self.data.swap_remove_and_drop_unchecked(row.index());
        self.added_ticks.swap_remove(row.index());
        self.changed_ticks.swap_remove(row.index());
    }

    // #[inline]
    // pub fn swap_remove_and_forget(&mut self, row:TableRow)->Option<(OwningPtr<'_>,)>
}
