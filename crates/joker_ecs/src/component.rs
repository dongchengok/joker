#![allow(unused)]

use std::{
    alloc::Layout,
    any::{Any, TypeId},
    borrow::{Borrow, Cow},
    cell::UnsafeCell,
    char::MAX,
    marker::PhantomData,
    mem::needs_drop,
};

use joker_foundation::HashMap;
use joker_ptr::{OwningPtr, UnsafeCellDeref};

use crate::{
    change_detection::MAX_CHANGE_AGE,
    storage::{SparseSetIndex, Storages},
    system::system_param::{Local, Resource},
    world::FromWorld, TypeIdMap,
};

pub trait Component: Send + Sync + 'static {
    type Storage: ComponentStorage;
}

pub struct TableStorage;

pub struct SparseStorage;

pub trait ComponentStorage: sealed::Sealed {
    const STORAGE_TYPE: StorageType;
}

impl ComponentStorage for TableStorage {
    const STORAGE_TYPE: StorageType = StorageType::Table;
}

impl ComponentStorage for SparseStorage {
    const STORAGE_TYPE: StorageType = StorageType::SparseSet;
}

mod sealed {
    pub trait Sealed {}
    impl Sealed for super::TableStorage {}
    impl Sealed for super::SparseStorage {}
}

#[derive(Debug, Default, Clone, Copy, PartialEq, Eq)]
pub enum StorageType {
    #[default]
    Table,
    SparseSet,
}

#[derive(Debug)]
pub struct ComponentInfo {
    id: ComponentId,
    descriptor: ComponentDescriptor,
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
        ComponentInfo { id, descriptor }
    }
}

#[derive(Debug, Clone, Copy, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct ComponentId(usize);

impl ComponentId {
    #[inline]
    pub const fn new(index: usize) -> Self {
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

pub struct ComponentDescriptor {
    name: Cow<'static, str>,
    storage_type: StorageType,
    is_send_and_sync: bool,
    type_id: Option<TypeId>,
    layout: Layout,
    drop: Option<for<'a> unsafe fn(OwningPtr<'a>)>,
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
            layout,
            drop,
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

    pub fn new_no_send<T: Any>(storage_type: StorageType) -> Self {
        Self {
            name: Cow::Borrowed(std::any::type_name::<T>()),
            storage_type: storage_type,
            is_send_and_sync: false,
            type_id: Some(TypeId::of::<T>()),
            layout: Layout::new::<T>(),
            drop: needs_drop::<T>().then_some(Self::drop_ptr::<T> as _),
        }
    }

    #[inline]
    pub fn storage_type(&self) -> StorageType {
        self.storage_type
    }

    #[inline]
    pub fn name(&self) -> &str {
        self.name.as_ref()
    }
}

#[derive(Debug, Default)]
pub struct Components {
    components: Vec<ComponentInfo>,
    indices: TypeIdMap<usize>,
    resource_indices: TypeIdMap<usize>,
}

impl Components {
    #[inline]
    pub fn init_component<T: Component>(&mut self, storages: &mut Storages) -> ComponentId {
        let type_id = TypeId::of::<T>();
        let Components {
            indices,
            components,
            ..
        } = self;
        let index = indices.entry(type_id).or_insert_with(|| {
            Components::init_component_inner(components, storages, ComponentDescriptor::new::<T>())
        });
        ComponentId(*index)
    }

    pub fn init_component_with_descriptor(
        &mut self,
        storages: &mut Storages,
        descriptor: ComponentDescriptor,
    ) -> ComponentId {
        let index = Components::init_component_inner(&mut self.components, storages, descriptor);
        ComponentId(index)
    }

    #[inline]
    pub fn init_component_inner(
        components: &mut Vec<ComponentInfo>,
        storages: &mut Storages,
        descriptor: ComponentDescriptor,
    ) -> usize {
        let index = components.len();
        let info = ComponentInfo::new(ComponentId(index), descriptor);
        if info.descriptor.storage_type == StorageType::SparseSet {
            storages.sparse_sets.get_or_insert(&info);
        }
        components.push(info);
        index
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.components.len()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.components.len() == 0
    }

    #[inline]
    pub fn get_info(&self, id: ComponentId) -> Option<&ComponentInfo> {
        self.components.get(id.0)
    }

    #[inline]
    pub fn get_name(&self, id: ComponentId) -> Option<&str> {
        self.get_info(id).map(|descriptor| descriptor.name())
    }

    #[inline]
    pub unsafe fn get_info_unchecked(&self, id: ComponentId) -> &ComponentInfo {
        debug_assert!(id.index() < self.components.len());
        self.components.get_unchecked(id.0)
    }

    #[inline]
    pub fn get_id(&self, type_id: TypeId) -> Option<ComponentId> {
        self.indices.get(&type_id).map(|index| ComponentId(*index))
    }

    #[inline]
    pub fn component_id<T: Component>(&self, type_id: TypeId) -> Option<ComponentId> {
        self.resource_indices
            .get(&type_id)
            .map(|index| ComponentId(*index))
    }

    #[inline]
    pub fn resource_id<T: Resource>(&mut self) -> ComponentId {
        unsafe {
            self.get_or_insert_resource_with(TypeId::of::<T>(), || {
                ComponentDescriptor::new_resource::<T>()
            })
        }
    }

    #[inline]
    pub fn init_non_send<T: Any>(&mut self) -> ComponentId {
        unsafe {
            self.get_or_insert_resource_with(TypeId::of::<T>(), || {
                ComponentDescriptor::new_no_send::<T>(StorageType::default())
            })
        }
    }

    #[inline]
    pub fn get_or_insert_resource_with(
        &mut self,
        type_id: TypeId,
        func: impl FnOnce() -> ComponentDescriptor,
    ) -> ComponentId {
        let components = &mut self.components;
        let index = self.resource_indices.entry(type_id).or_insert_with(|| {
            let descriptor = func();
            let index = components.len();
            components.push(ComponentInfo::new(ComponentId(index), descriptor));
            index
        });
        ComponentId(*index)
    }

    pub fn iter(&self) -> impl Iterator<Item = &ComponentInfo> + '_ {
        self.components.iter()
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Tick {
    tick: u32,
}

impl Tick {
    pub const MAX: Self = Self::new(MAX_CHANGE_AGE);

    #[inline]
    pub const fn new(tick: u32) -> Self {
        Self { tick }
    }

    #[inline]
    pub const fn get(self) -> u32 {
        self.tick
    }

    #[inline]
    pub fn set(&mut self, tick: u32) {
        self.tick = tick
    }

    #[inline]
    pub fn is_newer_than(self, last_run: Tick, this_run: Tick) -> bool {
        let ticks_since_insert = this_run.relative_to(self).tick.min(MAX_CHANGE_AGE);
        let ticks_since_system = this_run.relative_to(last_run).tick.min(MAX_CHANGE_AGE);
        ticks_since_system > ticks_since_insert
    }

    #[inline]
    pub fn relative_to(self, other: Self) -> Self {
        let tick = self.tick.wrapping_sub(other.tick);
        Self { tick }
    }

    #[inline]
    pub fn check_tick(&mut self, tick: Tick) -> bool {
        let age = tick.relative_to(*self);
        if age.get() > Self::MAX.get() {
            *self = tick.relative_to(Self::MAX);
            true
        } else {
            false
        }
    }
}

#[derive(Copy, Clone, Debug)]
pub struct TickCells<'a> {
    pub added: &'a UnsafeCell<Tick>,
    pub changed: &'a UnsafeCell<Tick>,
}

impl<'a> TickCells<'a> {
    #[inline]
    pub(crate) unsafe fn read(&self) -> ComponentTicks {
        ComponentTicks {
            added: self.added.read(),
            changed: self.changed.read(),
        }
    }
}

#[derive(Copy, Clone, Debug)]
pub struct ComponentTicks {
    pub(crate) added: Tick,
    pub(crate) changed: Tick,
}

impl ComponentTicks {
    #[inline]
    pub fn is_added(&self, last_run: Tick, this_run: Tick) -> bool {
        self.added.is_newer_than(last_run, this_run)
    }

    #[inline]
    pub fn is_changed(&self, last_run: Tick, this_run: Tick) -> bool {
        self.changed.is_newer_than(last_run, this_run)
    }

    pub(crate) fn new(change_tick: Tick) -> Self {
        Self {
            added: change_tick,
            changed: change_tick,
        }
    }

    #[inline]
    pub fn set_changed(&mut self, change_tick: Tick) {
        self.changed = change_tick
    }
}

pub struct ComponentIdFor<T: Component> {
    component_id: ComponentId,
    phantom: PhantomData<T>,
}

impl<T: Component> FromWorld for ComponentIdFor<T> {
    fn from_world(world: &mut crate::world::World) -> Self {
        Self {
            component_id: world.init_component::<T>(),
            phantom: PhantomData,
        }
    }
}

impl<T: Component> std::ops::Deref for ComponentIdFor<T> {
    type Target = ComponentId;
    fn deref(&self) -> &Self::Target {
        &self.component_id
    }
}

impl<T: Component> From<ComponentIdFor<T>> for ComponentId {
    fn from(to_component_id: ComponentIdFor<T>) -> Self {
        *to_component_id
    }
}

impl<'s, T: Component> From<Local<'s, ComponentIdFor<T>>> for ComponentId {
    fn from(to_component_id: Local<'s, ComponentIdFor<T>>) -> Self {
        **to_component_id
    }
}
