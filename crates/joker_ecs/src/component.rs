#![allow(unused)]

use std::{alloc::Layout, any::{TypeId, Any}, borrow::{Cow, Borrow}, mem::needs_drop};

use joker_ptr::OwningPtr;
use joker_foundation::HashMap;

use crate::{resource::Resource, storage::sparse_set::SparseSetIndex};

pub trait Component: Send + Sync + 'static {
    type Storage: ComponentStorage;
}

#[derive(Debug, Default, Clone, Copy, PartialEq, Eq)]
pub enum StorageType {
    #[default]
    Table,
    SparseSet,
}

pub struct TableStorage;

pub struct SparseStorage;

pub trait ComponentStorage: sealed::Sealed {
    const STORAGE_TYPE: StorageType;
}

mod sealed {
    pub trait Sealed {}
    impl Sealed for super::TableStorage {}
    impl Sealed for super::SparseStorage {}
}

#[derive(Debug, Clone, Copy, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct ComponentId(usize);

pub struct ComponentDescriptor {
    name: Cow<'static, str>,
    storage_type: StorageType,
    is_send_and_sync: bool,
    type_id: Option<TypeId>,
    layout: Layout,
    drop: Option<for<'a> unsafe fn(OwningPtr<'a>)>,
}

#[derive(Debug)]
pub struct ComponentInfo {
    id: ComponentId,
    descriptor: ComponentDescriptor,
}

type TypeIdMap<V> = HashMap<TypeId,V>;

#[derive(Debug,Default)]
pub struct Components{
    components:Vec<ComponentInfo>,
    indices:TypeIdMap<usize>,
    resource_indices:TypeIdMap<usize>,
}

impl ComponentStorage for TableStorage {
    const STORAGE_TYPE: StorageType = StorageType::Table;
}

impl ComponentStorage for SparseStorage {
    const STORAGE_TYPE: StorageType = StorageType::SparseSet;
}

impl ComponentId {
    #[inline]
    pub fn new(index: usize) -> Self {
        ComponentId(index)
    }

    #[inline]
    pub fn index(&self) -> usize {
        self.0
    }
}

impl SparseSetIndex for ComponentId {
    #[inline]
    fn sparse_set_index(&self) -> usize {
        self.0
    }

    #[inline]
    fn get_sparse_set_index(value: usize) -> Self {
        Self(value)
    }
}

impl ComponentInfo {
    #[inline]
    pub fn id(&self) -> ComponentId {
        self.id
    }

    #[inline]
    pub fn name(&self) -> &str {
        &self.descriptor.name
    }

    #[inline]
    pub fn type_id(&self) -> Option<TypeId> {
        self.descriptor.type_id
    }

    #[inline]
    pub fn layout(&self) -> Layout {
        self.descriptor.layout
    }

    pub fn drop(&self) -> Option<unsafe fn(OwningPtr<'_>)> {
        self.descriptor.drop
    }

    #[inline]
    pub fn storage_type(&self) -> StorageType {
        self.descriptor.storage_type
    }

    #[inline]
    pub fn is_send_and_sync(&self) -> bool {
        self.descriptor.is_send_and_sync
    }

    pub fn new(id: ComponentId, descriptor: ComponentDescriptor) -> Self {
        ComponentInfo {
            id: id,
            descriptor: descriptor,
        }
    }
}

impl std::fmt::Debug for ComponentDescriptor {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ComponentDescriptor")
            .field("name", &self.name)
            .field("storage_type", &self.storage_type)
            .field("is_send_and_sync", &self.is_send_and_sync)
            .field("layout", &self.layout)
            .finish()
    }
}

impl ComponentDescriptor {
    unsafe fn drop_ptr<T>(x: OwningPtr<'_>) {
        x.drop_as::<T>();
    }

    pub fn new<T: Component>() -> Self {
        Self {
            name: Cow::Borrowed(std::any::type_name::<T>()),
            storage_type: T::Storage::STORAGE_TYPE,
            is_send_and_sync: true,
            type_id: Some(TypeId::of::<T>()),
            layout: Layout::new::<T>(),
            drop: needs_drop::<T>().then_some(Self::drop_ptr::<T> as _),
        }
    }

    pub unsafe fn new_with_layout(
        name: impl Into<Cow<'static, str>>,
        storage_type: StorageType,
        layout: Layout,
        drop: Option<for<'a> unsafe fn(OwningPtr<'_>)>,
    ) -> Self {
        Self {
            name: name.into(),
            storage_type: storage_type,
            is_send_and_sync: true,
            type_id: None,
            layout: layout,
            drop: drop,
        }
    }

    pub fn new_resource<T: Resource>() -> Self {
        Self {
            name: Cow::Borrowed(std::any::type_name::<T>()),
            storage_type: StorageType::Table,
            is_send_and_sync: true,
            type_id: Some(TypeId::of::<T>()),
            layout: Layout::new::<T>(),
            drop: needs_drop::<T>().then_some(Self::drop_ptr::<T> as _),
        }
    }

    pub fn new_no_send<T:Any>(storage_type: StorageType)->Self{
        Self{
            name:Cow::Borrowed(std::any::type_name::<T>()),
            storage_type:storage_type,
            is_send_and_sync:false,
            type_id:Some(TypeId::of::<T>()),
            layout:Layout::new::<T>(),
            drop:needs_drop::<T>().then_some(Self::drop_ptr::<T> as _)
        }
    }

    #[inline]
    pub fn storage_type(&self)->StorageType{
        self.storage_type
    }

    #[inline]
    pub fn name(&self)->&str{
        self.name.as_ref()
    }
}

// impl Components{
//     #[inline]
//     pub fn init_component<T:Component>(&mut self, storage:&mut Storages)
// }

#[cfg(test)]
mod tests {
    use super::ComponentId;

    #[test]
    fn test_any_type_name() {
        let name = std::any::type_name::<ComponentId>();
        assert_eq!(
            name.to_string(),
            "joker_ecs::component::ComponentId".to_string()
        );
    }
}
