#![allow(unused)]

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableId(u32);

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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableRow(u32);

impl TableRow {
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
