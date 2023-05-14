#![allow(unused)]

use std::hash::Hash;
use std::marker::PhantomData;

use crate::component::ComponentId;
use crate::entity::Entity;

use super::table::Column;

type EntityIndex = u32;

#[derive(Debug)]
pub struct SparseArray<I, V = I> {
    values: Vec<Option<V>>,
    marker: PhantomData<I>,
}

#[derive(Debug)]
pub struct ImmutableSparseArray<I, V = I> {
    values: Box<[Option<V>]>,
    marker: PhantomData<I>,
}

#[derive(Debug)]
pub struct SparseSet<I, V: 'static> {
    dense: Vec<V>,
    indices: Vec<I>,
    sparse: SparseArray<I, usize>,
}

#[derive(Debug)]
pub struct ImmutableSparseSet<I,V:'static>{
    dense:Box<V>,
    indices:Box<[I]>,
    sparse:ImmutableSparseArray<I,usize>,
}

pub struct ComponentSparseSet{
    dense:Column,
    #[cfg(not(debug_assertions))]
    entities:Vec<EntityIndex>,
    #[cfg(debug_assertions)]
    entities:Vec<Entity>,
    sparse:SparseArray<EntityIndex,u32>,
}

pub struct SparseSets {
    sets: SparseSet<ComponentId, ComponentSparseSet>,
}

pub trait SparseSetIndex: Clone + PartialEq + Eq + Hash {
    fn sparse_set_index(&self) -> usize;
    fn get_sparse_set_index(value: usize) -> Self;
}

impl<I: SparseSetIndex, V> Default for SparseArray<I, V> {
    fn default() -> Self {
        Self::new()
    }
}

impl<I, V> SparseArray<I, V> {
    #[inline]
    pub const fn new() -> Self {
        Self {
            values: Vec::new(),
            marker: PhantomData,
        }
    }
}

macro_rules! impl_sparse_array {
    ($ty:ident) => {
        impl<I: SparseSetIndex, V> $ty<I, V> {
            #[inline]
            pub fn contains(&self, index: I) -> bool {
                let index = index.sparse_set_index();
                self.values.get(index).map(|v| v.is_some()).unwrap_or(false)
            }

            #[inline]
            pub fn get(&self, index: I) -> Option<&V> {
                let index = index.sparse_set_index();
                self.values.get(index).map(|v| v.as_ref()).unwrap_or(None)
            }
        }
    };
}
impl_sparse_array!(SparseArray);
impl_sparse_array!(ImmutableSparseArray);

impl<I: SparseSetIndex, V> SparseArray<I, V> {
    #[inline]
    pub fn insert(&mut self, index: I, value: V) {
        let index = index.sparse_set_index();
        if index >= self.values.len() {
            self.values.resize_with(index + 1, || None);
        }
        self.values[index] = Some(value);
    }

    #[inline]
    pub fn get_mut(&mut self, index: I) -> Option<&mut V> {
        let index = index.sparse_set_index();
        self.values
            .get_mut(index)
            .map(|v| v.as_mut())
            .unwrap_or(None)
    }

    //只移除内容，不位移数组
    #[inline]
    pub fn remove(&mut self, index: I) -> Option<V> {
        let index = index.sparse_set_index();
        self.values.get_mut(index).and_then(|value| value.take())
    }

    pub fn clear(&mut self) {
        self.values.clear()
    }

    pub fn into_immutable(self) -> ImmutableSparseArray<I, V> {
        ImmutableSparseArray {
            values: self.values.into_boxed_slice(),
            marker: PhantomData,
        }
    }
}
