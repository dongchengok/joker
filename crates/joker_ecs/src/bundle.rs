#![allow(unused)]

use joker_ptr::OwningPtr;

use crate::{
    component::{ComponentId, Components, StorageType},
    storage::{SparseSetIndex, Storages},
};

pub unsafe trait Bundle: DynamicBundle + Send + Sync + 'static {
    fn component_ids(
        components: &mut Components,
        storage: &mut Storages,
        ids: &mut impl FnMut(ComponentId),
    );
}

pub trait DynamicBundle {
    fn get_components(self, func: &mut impl FnMut(StorageType, OwningPtr<'_>));
}

// unsafe impl<C:Component> Bundle for C{
//     fn component_ids(
//             components: &mut Components,
//             storages: &mut Storages,
//             ids: &mut impl FnMut(ComponentId),
//         ) {
//         ids(components.init_component::<C>(storages));
//     }
// }
#[derive(Debug, Clone, Copy, Eq, PartialEq, Hash)]
pub struct BundleId(usize);

impl BundleId {
    #[inline]
    pub fn index(self) -> usize {
        self.0
    }
}

impl SparseSetIndex for BundleId {
    #[inline]
    fn sparse_set_index(&self) -> usize {
        self.index()
    }

    #[inline]
    fn get_sparse_set_index(value: usize) -> Self {
        Self(value)
    }
}
