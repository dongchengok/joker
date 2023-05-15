#![allow(unused)]

use joker_ptr::OwningPtr;

use crate::{component::{StorageType, ComponentId, Components}, storage::Storages};

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