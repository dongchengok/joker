#![allow(unused)]

use crate::storage::SparseSetIndex;
use fixedbitset::FixedBitSet;
use std::fmt;
use std::marker::PhantomData;

struct FormattedBitSet<'a, T: SparseSetIndex> {
    bit_set: &'a FixedBitSet,
    _marker: PhantomData<T>,
}

impl<'a, T: SparseSetIndex> FormattedBitSet<'a, T> {
    fn new(bit_set: &'a FixedBitSet) -> Self {
        Self {
            bit_set,
            _marker: PhantomData,
        }
    }
}

impl<'a, T: SparseSetIndex + fmt::Debug> fmt::Debug for FormattedBitSet<'a, T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_list()
            .entries(self.bit_set.ones().map(T::get_sparse_set_index))
            .finish()
    }
}

#[derive(Clone, Eq, PartialEq)]
pub struct Access<T: SparseSetIndex> {
    reads_and_writes: FixedBitSet,
    writes: FixedBitSet,
    reads_all: bool,
    marker: PhantomData<T>,
}

impl<T: SparseSetIndex + fmt::Debug> fmt::Debug for Access<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("Access")
            .field(
                "reads_and_writes",
                &FormattedBitSet::<T>::new(&self.reads_and_writes),
            )
            .field("writes", &FormattedBitSet::<T>::new(&self.writes))
            .field("reads_all", &self.reads_all)
            .finish()
    }
}

impl<T: SparseSetIndex> Default for Access<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T: SparseSetIndex> Access<T> {
    pub const fn new() -> Self {
        Self {
            reads_and_writes: FixedBitSet::new(),
            writes: FixedBitSet::new(),
            reads_all: false,
            marker: PhantomData,
        }
    }

    #[inline]
    pub fn grow(&mut self, capacity: usize) {
        self.reads_and_writes.grow(capacity);
        self.writes.grow(capacity);
    }

    #[inline]
    pub fn add_read(&mut self, index: T) {
        self.reads_and_writes.grow(index.sparse_set_index() + 1);
        self.reads_and_writes.insert(index.sparse_set_index());
    }

    #[inline]
    pub fn add_write(&mut self, index: T) {
        self.reads_and_writes.grow(index.sparse_set_index() + 1);
        self.reads_and_writes.insert(index.sparse_set_index());
        self.writes.grow(index.sparse_set_index() + 1);
        self.writes.insert(index.sparse_set_index());
    }

    #[inline]
    pub fn has_read(&self, index: T) -> bool {
        self.reads_all || self.reads_and_writes.contains(index.sparse_set_index())
    }

    #[inline]
    pub fn has_read_all(&self)->bool{
        self.reads_all
    }

    #[inline]
    pub fn clear(&mut self){
        self.reads_all = false;
        self.reads_and_writes.clear();
        self.writes.clear();
    }
}
