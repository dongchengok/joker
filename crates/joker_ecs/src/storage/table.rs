#![allow(unused)]

use joker_ptr::{OwningPtr, Ptr, PtrMut, UnsafeCellDeref};

use super::blob_vec::BlobVec;
use crate::component::{ComponentInfo, ComponentTick, ComponentTicks, Tick};
use std::{alloc::Layout, cell::UnsafeCell};

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
    pub fn item_layout(&self) -> Layout {
        self.data.layout()
    }

    #[inline]
    pub unsafe fn initialize(&mut self, row: TableRow, data: OwningPtr<'_>, tick: Tick) {
        debug_assert!(row.index() < self.len());
        self.data.initialize_unchecked(row.index(), data);
        *self.added_ticks.get_unchecked_mut(row.index()).get_mut() = tick;
        *self.changed_ticks.get_unchecked_mut(row.index()).get_mut() = tick;
    }

    #[inline]
    pub unsafe fn replace(&mut self, row: TableRow, data: OwningPtr<'_>, change_tick: Tick) {
        debug_assert!(row.index() < self.len());
        self.data.replace_unchecked(row.index(), data);
        *self.changed_ticks.get_unchecked_mut(row.index()).get_mut() = change_tick;
    }

    #[inline]
    pub unsafe fn replace_untracked(&mut self, row: TableRow, data: OwningPtr<'_>) {
        debug_assert!(row.index() < self.len());
        self.data.replace_unchecked(row.index(), data);
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.data.len()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    #[inline]
    pub unsafe fn swap_remove_unchecked(&mut self, row: TableRow) {
        self.data.swap_remove_and_drop_unchecked(row.index());
        self.added_ticks.swap_remove(row.index());
        self.changed_ticks.swap_remove(row.index());
    }

    #[inline]
    #[must_use = "返回值必须被使用，要不内存就丢了"]
    pub(crate) fn swap_remove_and_forget(
        &mut self,
        row: TableRow,
    ) -> Option<(OwningPtr<'_>, ComponentTicks)> {
        (row.index() < self.data.len()).then(|| {
            let data = unsafe { self.data.swap_remove_and_forget_unchecked(row.index()) };
            let added = self.added_ticks.swap_remove(row.index()).into_inner();
            let changed = self.changed_ticks.swap_remove(row.index()).into_inner();
            (data, ComponentTicks { added, changed })
        })
    }

    #[inline]
    #[must_use = "返回值必须被使用，否则内存就丢了"]
    pub(crate) unsafe fn swap_remove_and_forget_unchecked(
        &mut self,
        row: TableRow,
    ) -> (OwningPtr<'_>, ComponentTicks) {
        let data = self.data.swap_remove_and_forget_unchecked(row.index());
        let added = self.added_ticks.swap_remove(row.index()).into_inner();
        let changed = self.changed_ticks.swap_remove(row.index()).into_inner();
        (data, ComponentTicks { added, changed })
    }

    pub(crate) unsafe fn push(&mut self, ptr: OwningPtr<'_>, ticks: ComponentTicks) {
        self.data.push(ptr);
        self.added_ticks.push(UnsafeCell::new(ticks.added));
        self.changed_ticks.push(UnsafeCell::new(ticks.changed));
    }

    #[inline]
    pub unsafe fn get_data_unchecked(&self, row: TableRow) -> Ptr<'_> {
        debug_assert!(row.index() < self.data.len());
        self.data.get_unchecked(row.index())
    }

    #[inline]
    pub fn get_data_mut(&mut self, row: TableRow) -> Option<PtrMut<'_>> {
        (row.index() < self.data.len()).then(|| unsafe { self.data.get_unchecked_mut(row.index()) })
    }

    #[inline]
    pub unsafe fn get_added_ticks_unchecked(&self, row: TableRow) -> &UnsafeCell<Tick> {
        debug_assert!(row.index() < self.added_ticks.len());
        self.added_ticks.get_unchecked(row.index())
    }

    #[inline]
    pub unsafe fn get_changed_ticks_unchecked(&self, row: TableRow) -> &UnsafeCell<Tick> {
        debug_assert!(row.index() < self.changed_ticks.len());
        self.changed_ticks.get_unchecked(row.index())
    }

    #[inline]
    pub unsafe fn get_ticks_unchecked(&self, row: TableRow) -> ComponentTicks {
        debug_assert!(row.index() < self.added_ticks.len());
        debug_assert!(row.index() < self.changed_ticks.len());
        ComponentTicks {
            added: self.added_ticks.get_unchecked(row.index()).read(),
            changed: self.changed_ticks.get_unchecked(row.index()).read(),
        }
    }

    pub fn clear(&mut self) {
        self.data.clear();
        self.added_ticks.clear();
        self.changed_ticks.clear();
    }

    #[inline]
    pub(crate) fn check_change_ticks(&mut self, change_tick: Tick) {
        for component_ticks in &mut self.added_ticks {
            component_ticks.get_mut().check_tick(change_tick);
        }
        for component_ticks in &mut self.changed_ticks {
            component_ticks.get_mut().check_tick(change_tick);
        }
    }
}
