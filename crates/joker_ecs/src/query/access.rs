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
    pub fn has_write(&self, index: T) -> bool {
        self.writes.contains(index.sparse_set_index())
    }

    #[inline]
    pub fn read_all(&mut self) {
        self.reads_all = true;
    }

    #[inline]
    pub fn has_read_all(&self) -> bool {
        self.reads_all
    }

    #[inline]
    pub fn clear(&mut self) {
        self.reads_all = false;
        self.reads_and_writes.clear();
        self.writes.clear();
    }

    pub fn extend(&mut self, other: &Access<T>) {
        self.reads_all = self.reads_all || other.reads_all;
        self.reads_and_writes.union_with(&other.reads_and_writes);
        self.writes.union_with(&other.writes);
    }

    pub fn is_compatible(&self, other: &Access<T>) -> bool {
        if self.reads_all {
            return other.writes.count_ones(..) == 0;
        }
        if other.reads_all {
            return self.writes.count_ones(..) == 0;
        }
        self.writes.is_disjoint(&other.reads_and_writes)
            && other.writes.is_disjoint(&self.reads_and_writes)
    }

    pub fn get_conflicts(&self, other: &Access<T>) -> Vec<T> {
        let mut conflicts = FixedBitSet::default();
        if self.reads_all {
            conflicts.extend(other.writes.ones());
        }
        if other.reads_all {
            conflicts.extend(self.writes.ones());
        }
        conflicts.extend(self.writes.intersection(&other.reads_and_writes));
        conflicts.extend(self.reads_and_writes.intersection(&other.writes));
        conflicts
            .ones()
            .map(SparseSetIndex::get_sparse_set_index)
            .collect()
    }

    pub fn reads_and_writes(&self) -> impl Iterator<Item = T> + '_ {
        self.reads_and_writes.ones().map(T::get_sparse_set_index)
    }

    pub fn writes(&self) -> impl Iterator<Item = T> + '_ {
        self.writes.ones().map(T::get_sparse_set_index)
    }
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct FilteredAccess<T: SparseSetIndex> {
    access: Access<T>,
    filter_sets: Vec<AccessFilters<T>>,
}

impl<T: SparseSetIndex> Default for FilteredAccess<T> {
    fn default() -> Self {
        Self {
            access: Access::default(),
            filter_sets: vec![AccessFilters::default()],
        }
    }
}

impl<T: SparseSetIndex> From<FilteredAccess<T>> for FilteredAccessSet<T> {
    fn from(filtered_access: FilteredAccess<T>) -> Self {
        let mut base = FilteredAccessSet::<T>::default();
        base.add(filtered_access);
        base
    }
}

impl<T: SparseSetIndex> FilteredAccess<T> {
    #[inline]
    pub fn access(&self) -> &Access<T> {
        &self.access
    }

    #[inline]
    pub fn access_mut(&mut self) -> &mut Access<T> {
        &mut self.access
    }

    pub fn add_read(&mut self, index: T) {
        self.access.add_read(index.clone());
        self.and_with(index);
    }

    pub fn add_write(&mut self, index: T) {
        self.access.add_write(index.clone());
        self.and_with(index);
    }

    pub fn and_with(&mut self, index: T) {
        let index = index.sparse_set_index();
        for filter in &mut self.filter_sets {
            filter.with.grow(index + 1);
            filter.with.insert(index);
        }
    }

    pub fn add_without(&mut self, index: T) {
        let index = index.sparse_set_index();
        for filter in &mut self.filter_sets {
            filter.without.grow(index + 1);
            filter.without.insert(index);
        }
    }

    pub fn append_or(&mut self, other: &FilteredAccess<T>) {
        self.filter_sets.append(&mut self.filter_sets.clone());
    }

    pub fn extend_access(&mut self, other: &FilteredAccess<T>) {
        self.access.extend(&other.access)
    }

    pub fn is_compatible(&self, other: &FilteredAccess<T>) -> bool {
        if self.access.is_compatible(&other.access) {
            return true;
        }
        self.filter_sets.iter().all(|filter| {
            other
                .filter_sets
                .iter()
                .all(|other_filter| filter.is_ruled_out_by(other_filter))
        })
    }

    pub fn get_conflicts(&self, other: &FilteredAccess<T>) -> Vec<T> {
        if !self.is_compatible(other) {
            return self.access.get_conflicts(&other.access);
        }
        Vec::new()
    }

    pub fn extend(&mut self, other: &FilteredAccess<T>) {
        self.access.extend(&other.access);

        if other.filter_sets.len() == 1 {
            for filter in &mut self.filter_sets {
                filter.with.union_with(&other.filter_sets[0].with);
                filter.without.union_with(&other.filter_sets[0].without);
            }
            return;
        }

        let mut new_filters = Vec::with_capacity(self.filter_sets.len() * other.filter_sets.len());
        for filter in &self.filter_sets {
            for other_filter in &other.filter_sets {
                let mut new_filter = filter.clone();
                new_filter.with.union_with(&other_filter.with);
                new_filter.without.union_with(&other_filter.without);
                new_filters.push(new_filter);
            }
        }
        self.filter_sets = new_filters;
    }

    pub fn read_all(&mut self) {
        self.access.read_all();
    }
}

#[derive(Clone, Eq, PartialEq)]
struct AccessFilters<T> {
    with: FixedBitSet,
    without: FixedBitSet,
    _index_type: PhantomData<T>,
}

impl<T: SparseSetIndex + fmt::Debug> fmt::Debug for AccessFilters<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("AccessFilters")
            .field("with", &FormattedBitSet::<T>::new(&self.with))
            .field("without", &FormattedBitSet::<T>::new(&self.without))
            .finish()
    }
}

impl<T: SparseSetIndex> Default for AccessFilters<T> {
    fn default() -> Self {
        Self {
            with: FixedBitSet::default(),
            without: FixedBitSet::default(),
            _index_type: PhantomData,
        }
    }
}

impl<T: SparseSetIndex> AccessFilters<T> {
    fn is_ruled_out_by(&self, other: &Self) -> bool {
        !self.with.is_disjoint(&other.without) || !self.without.is_disjoint(&other.with)
    }
}

#[derive(Debug, Clone)]
pub struct FilteredAccessSet<T: SparseSetIndex> {
    combined_access: Access<T>,
    filtered_accesses: Vec<FilteredAccess<T>>,
}

impl<T: SparseSetIndex> FilteredAccessSet<T> {
    pub fn add(&mut self, filtered_access: FilteredAccess<T>) {
        self.combined_access.extend(&filtered_access.access);
        self.filtered_accesses.push(filtered_access);
    }
}

impl<T: SparseSetIndex> Default for FilteredAccessSet<T> {
    fn default() -> Self {
        Self {
            combined_access: Default::default(),
            filtered_accesses: Vec::new(),
        }
    }
}
