#![allow(unused)]

use std::hash::Hash;
use std::marker::PhantomData;

use crate::component::{ComponentId, ComponentInfo};
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

#[derive(Default)]
pub struct SparseSets {
    sets: SparseSet<ComponentId, ComponentSparseSet>,
}

#[derive(Debug)]
pub struct ImmutableSparseSet<I, V: 'static> {
    dense: Box<[V]>,
    indices: Box<[I]>,
    sparse: ImmutableSparseArray<I, usize>,
}

pub struct ComponentSparseSet {
    dense: Column,
    #[cfg(not(debug_assertions))]
    entities: Vec<EntityIndex>,
    #[cfg(debug_assertions)]
    entities: Vec<Entity>,
    sparse: SparseArray<EntityIndex, u32>,
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

macro_rules! impl_sparse_set {
    ($ty:ident) => {
        impl<I: SparseSetIndex, V> $ty<I, V> {
            #[inline]
            pub fn len(&self) -> usize {
                self.dense.len()
            }

            #[inline]
            pub fn contains(&self, index: I) -> bool {
                self.sparse.contains(index)
            }

            pub fn get(&self, index: I) -> Option<&V> {
                self.sparse
                    .get(index)
                    .map(|dense_index| unsafe { self.dense.get_unchecked(*dense_index) })
            }

            pub fn get_mut(&mut self, index: I) -> Option<&mut V> {
                let dense = &mut self.dense;
                self.sparse
                    .get(index)
                    .map(move |dense_index| unsafe { dense.get_unchecked_mut(*dense_index) })
            }

            pub fn indices(&self) -> impl Iterator<Item = I> + '_ {
                self.indices.iter().cloned()
            }

            pub fn values(&self) -> impl Iterator<Item = &V> {
                self.dense.iter()
            }

            pub fn values_mut(&mut self) -> impl Iterator<Item = &mut V> {
                self.dense.iter_mut()
            }

            pub fn iter(&self) -> impl Iterator<Item = (&I, &V)> {
                self.indices.iter().zip(self.dense.iter())
            }

            pub fn iter_mut(&mut self) -> impl Iterator<Item = (&I, &mut V)> {
                self.indices.iter().zip(self.dense.iter_mut())
            }
        }
    };
}

impl_sparse_set!(SparseSet);
impl_sparse_set!(ImmutableSparseSet);

impl<I: SparseSetIndex, V> Default for SparseSet<I, V> {
    fn default() -> Self {
        Self::new()
    }
}

impl<I, V> SparseSet<I, V> {
    pub const fn new() -> Self {
        Self {
            dense: Vec::new(),
            indices: Vec::new(),
            sparse: SparseArray::new(),
        }
    }
}

impl<I: SparseSetIndex, V> SparseSet<I, V> {
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            dense: Vec::with_capacity(capacity),
            indices: Vec::with_capacity(capacity),
            sparse: Default::default(),
        }
    }

    #[inline]
    pub fn capacity(&self) -> usize {
        self.dense.capacity()
    }

    pub fn insert(&mut self, index: I, value: V) {
        if let Some(dense_index) = self.sparse.get(index.clone()).cloned() {
            unsafe {
                *self.dense.get_unchecked_mut(dense_index) = value;
            }
        } else {
            self.sparse.insert(index.clone(), self.dense.len());
            self.indices.push(index);
            self.dense.push(value);
        }
    }

    pub fn get_or_insert_with(&mut self, index: I, func: impl FnOnce() -> V) -> &mut V {
        if let Some(dense_index) = self.sparse.get(index.clone()).cloned() {
            unsafe { self.dense.get_unchecked_mut(dense_index) }
        } else {
            let value = func();
            let dense_index = self.dense.len();
            self.sparse.insert(index.clone(), dense_index);
            self.indices.push(index);
            self.dense.push(value);
            unsafe { self.dense.get_unchecked_mut(dense_index) }
        }
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.dense.len() == 0
    }

    pub fn remove(&mut self, index: I) -> Option<V> {
        todo!("这里不太对吧，删除最后一个sparse的索引关系被遗留了下来，如果再娶的话有问题吧，或者是考虑没有重新分配，所以没问题？");
        self.sparse.remove(index).map(|dense_index| {
            let is_last = dense_index == (self.dense.len() - 1);
            let value = self.dense.swap_remove(dense_index);
            self.indices.swap_remove(dense_index);
            if !is_last {
                let swapped_index = self.indices[dense_index].clone();
                *self.sparse.get_mut(swapped_index).unwrap() = dense_index;
            }
            value
        })
    }

    pub fn clear(&mut self) {
        self.dense.clear();
        self.indices.clear();
        self.sparse.clear();
    }

    pub fn into_immutable(self) -> ImmutableSparseSet<I, V> {
        ImmutableSparseSet {
            dense: self.dense.into_boxed_slice(),
            indices: self.indices.into_boxed_slice(),
            sparse: self.sparse.into_immutable(),
        }
    }
}

macro_rules! impl_sparse_set_index {
    ($($ty:ty),+) => {
        $(impl SparseSetIndex for $ty{
            #[inline]
            fn sparse_set_index(&self)->usize{
                *self as usize
            }

            #[inline]
            fn get_sparse_set_index(value:usize)->Self{
                value as $ty
            }
        })*
    };
}
impl_sparse_set_index!(u8, u16, u32, u64, usize);

impl ComponentSparseSet{
    // pub fn new(component_info:&ComponentInfo, capacity:usize)->Self{
    //     Self { dense: Column::, entities: (), sparse: () }
    // }
}

impl SparseSets {
    #[inline]
    pub fn len(&self) -> usize {
        self.sets.len()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.sets.is_empty()
    }

    #[inline]
    pub fn iter(&self) -> impl Iterator<Item = (ComponentId, &ComponentSparseSet)> {
        self.sets.iter().map(|(id, data)| (*id, data))
    }

    #[inline]
    pub fn get(&self, comonent_id: ComponentId) -> Option<&ComponentSparseSet> {
        self.sets.get(comonent_id)
    }

    // pub fn get_or_insert(&mut self, component_info: &ComponentInfo) -> &mut ComponentSparseSet {
    //     if !self.sets.contains(component_info.id()) {
    //         self.sets.insert(
    //             component_info.id(),
    //             ComponentSparseSet::new(component_info, 64),
    //         );
    //     }
    //     self.sets.get_mut(component_info.id()).unwrap()
    // }
}
