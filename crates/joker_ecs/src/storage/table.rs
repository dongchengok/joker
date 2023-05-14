#![allow(unused)]

use std::cell::UnsafeCell;
use crate::component::Tick;
use super::blob_vec::BlobVec;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableId(u32);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableRow(u32);

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

// #[derive(Debug)]
// pub struct Column{
//     data:BlobVec;
// }
