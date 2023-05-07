use std::hash::Hash;
use std::marker::PhantomData;

type EntityIndex = u32;

pub trait SparseSetIndex: Clone + PartialEq + Eq + Hash {
    fn sparse_set_index(&self) -> usize;
    fn get_sparse_set_index(value: usize) -> Self;
}

#[derive(Debug)]
pub struct SparseArray<I, V = I> {
    values: Vec<Option<V>>,
    marker: PhantomData<I>,
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

impl<I: SparseSetIndex, V> Default for SparseArray<I, V> {
    fn default() -> Self {
        Self::new()
    }
}

#[derive(Debug)]
pub struct ImmutableSparseArray<I, V = I> {
    values: Box<[Option<V>]>,
    marker: PhantomData<I>,
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
