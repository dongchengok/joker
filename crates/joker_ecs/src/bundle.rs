#![allow(unused)]
use std::any::TypeId;

use joker_ptr::OwningPtr;

use joker_foundation::{all_tuples, HashMap, HashSet};

use crate::{
    archetype::{
        self, Archetype, ArchetypeId, Archetypes, BundleComponentStatus, ComponentStatus,
        SpawnBundleStatus,
    },
    component::{Component, ComponentId, ComponentStorage, Components, StorageType, Tick},
    entity::{Entities, Entity, EntityLocation},
    query::DebugCheckedUnwrap,
    storage::{SparseSetIndex, SparseSets, Storages, Table, TableRow},
    TypeIdMap,
};

pub unsafe trait Bundle: DynamicBundle + Send + Sync + 'static {
    fn component_ids(
        components: &mut Components,
        storages: &mut Storages,
        ids: &mut impl FnMut(ComponentId),
    );

    unsafe fn from_components<T, F>(ctx: &mut T, func: &mut F) -> Self
    where
        F: for<'a> FnMut(&'a mut T) -> OwningPtr<'a>,
        Self: Sized;
}

pub trait DynamicBundle {
    fn get_components(self, func: &mut impl FnMut(StorageType, OwningPtr<'_>));
}

unsafe impl<C: Component> Bundle for C {
    fn component_ids(
        components: &mut Components,
        storage: &mut Storages,
        ids: &mut impl FnMut(ComponentId),
    ) {
        ids(components.init_component::<C>(storage));
    }

    unsafe fn from_components<T, F>(ctx: &mut T, func: &mut F) -> Self
    where
        F: for<'a> FnMut(&'a mut T) -> OwningPtr<'a>,
        Self: Sized,
    {
        func(ctx).read()
    }
}

impl<C: Component> DynamicBundle for C {
    #[inline]
    fn get_components(self, func: &mut impl FnMut(StorageType, OwningPtr<'_>)) {
        OwningPtr::make(self, |ptr| func(C::Storage::STORAGE_TYPE, ptr));
    }
}

macro_rules! tuple_impl {
    ($($name:ident),*) => {
        unsafe impl<$($name:Bundle),*> Bundle for ($($name,)*){
            #[allow(unused_variables)]
            fn component_ids(components:&mut Components, storages:&mut Storages, ids: &mut impl FnMut(ComponentId)){
                $(<$name as Bundle>::component_ids(components, storages, ids);)*
            }

            #[allow(unused_variables, unused_mut)]
            #[allow(clippy::unused_unit)]
            unsafe fn from_components<T,F>(ctx: &mut T, func: &mut F)->Self
            where
                F: FnMut(&mut T)->OwningPtr<'_>
                {
                    ($(<$name as Bundle>::from_components(ctx, func),)*)
                }
        }

        impl<$($name:Bundle),*> DynamicBundle for ($($name,)*){
            #[allow(unused_variables,unused_mut)]
            #[inline(always)]
            fn get_components(self, func:&mut impl FnMut(StorageType, OwningPtr<'_>)){
                #[allow(non_snake_case)]
                let ($(mut $name,)*) = self;
                $(
                    $name.get_components(&mut *func);
                )*
            }
        }
    };
}

all_tuples!(tuple_impl, 0, 15, B);

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

pub struct BundleInfo {
    id: BundleId,
    component_ids: Vec<ComponentId>,
}

impl BundleInfo {
    unsafe fn new(
        bundle_type_name: &'static str,
        components: &Components,
        component_ids: Vec<ComponentId>,
        id: BundleId,
    ) -> BundleInfo {
        let mut deduped = component_ids.clone();
        deduped.sort();
        deduped.dedup();

        if deduped.len() != component_ids.len() {
            let mut seen = HashSet::new();
            let mut dups = Vec::new();
            for id in component_ids {
                if !seen.insert(id) {
                    dups.push(id);
                }
            }

            let names = dups
                .into_iter()
                .map(|id| unsafe { components.get_info_unchecked(id).name() })
                .collect::<Vec<_>>()
                .join(", ");

            panic!("Bundle {bundle_type_name} has duplicate components: {names}");
        }

        BundleInfo { id, component_ids }
    }

    #[inline]
    pub const fn id(&self) -> BundleId {
        self.id
    }

    #[inline]
    pub fn components(&self) -> &[ComponentId] {
        &self.component_ids
    }

    pub(crate) fn get_bundle_inserter<'a, 'b>(
        &'b self,
        entities: &'a mut Entities,
        archetypes: &'a mut Archetypes,
        components: &mut Components,
        storages: &'a mut Storages,
        archetype_id: ArchetypeId,
        change_tick: Tick,
    ) -> BundleInserter<'a, 'b> {
        let new_archetype_id =
            self.add_bundle_to_archetype(archetypes, storages, components, archetype_id);
        let archetypes_ptr = archetypes.archetypes.as_mut_ptr();
        if new_archetype_id == archetype_id {
            let archetype = &mut archetypes[archetype_id];
            let table_id = archetype.table_id();
            BundleInserter {
                bundle_info: self,
                archetype,
                entities,
                sparse_sets: &mut storages.sparse_sets,
                table: &mut storages.tables[table_id],
                archetypes_ptr,
                change_tick,
                result: InsertBundleResult::SameArchetype,
            }
        } else {
            let (archetype, new_archetype) = archetypes.get_2_mut(archetype_id, new_archetype_id);
            let table_id = archetype.table_id();
            if table_id == new_archetype.table_id() {
                BundleInserter {
                    bundle_info: self,
                    archetype,
                    archetypes_ptr,
                    entities,
                    sparse_sets: &mut storages.sparse_sets,
                    table: &mut storages.tables[table_id],
                    change_tick,
                    result: InsertBundleResult::NewArchetypeSameTable { new_archetype },
                }
            } else {
                let (table, new_table) = storages
                    .tables
                    .get_2_mut(table_id, new_archetype.table_id());
                BundleInserter {
                    bundle_info: self,
                    archetype,
                    sparse_sets: &mut storages.sparse_sets,
                    entities,
                    archetypes_ptr,
                    table,
                    change_tick,
                    result: InsertBundleResult::NewArchetypeNewTable {
                        new_archetype,
                        new_table,
                    },
                }
            }
        }
    }

    pub(crate) fn get_bundle_spawner<'a, 'b>(
        &'b self,
        entities: &'a mut Entities,
        archetypes: &'a mut Archetypes,
        components: &mut Components,
        storages: &'a mut Storages,
        change_tick: Tick,
    ) -> BundleSpawner<'a, 'b> {
        let new_archetype_id =
            self.add_bundle_to_archetype(archetypes, storages, components, ArchetypeId::EMPTY);
        let archetype = &mut archetypes[new_archetype_id];
        let table = &mut storages.tables[archetype.table_id()];
        BundleSpawner {
            archetype,
            entities,
            bundle_info: self,
            table,
            sparse_sets: &mut storages.sparse_sets,
            change_tick,
        }
    }

    #[inline]
    #[allow(clippy::too_many_arguments)]
    unsafe fn write_components<T: DynamicBundle, S: BundleComponentStatus>(
        &self,
        table: &mut Table,
        sparse_sets: &mut SparseSets,
        bundle_component_status: &S,
        entity: Entity,
        table_row: TableRow,
        change_tick: Tick,
        bundle: T,
    ) {
        let mut bundle_component = 0;
        bundle.get_components(&mut |storage_type, component_ptr| {
            let component_id = *self.component_ids.get_unchecked(bundle_component);
            match storage_type {
                StorageType::Table => {
                    let column =
                        unsafe { table.get_column_mut(component_id).debug_checked_unwrap() };
                    match bundle_component_status.get_status(bundle_component) {
                        ComponentStatus::Add => {
                            column.initialize(table_row, component_ptr, change_tick);
                        }
                        ComponentStatus::Mutated => {
                            column.replace(table_row, component_ptr, change_tick);
                        }
                    }
                }
                StorageType::SparseSet => {
                    let sparse_set =
                        unsafe { sparse_sets.get_mut(component_id).debug_checked_unwrap() };
                    sparse_set.insert(entity, component_ptr, change_tick);
                }
            }
            bundle_component += 1;
        });
    }

    pub(crate) fn add_bundle_to_archetype(
        &self,
        archetypes: &mut Archetypes,
        storages: &mut Storages,
        components: &mut Components,
        archetype_id: ArchetypeId,
    ) -> ArchetypeId {
        if let Some(add_bundle_id) = archetypes[archetype_id].edges().get_add_bundle(self.id) {
            return add_bundle_id;
        }
        let mut new_table_components = Vec::new();
        let mut new_sparse_set_components = Vec::new();
        let mut bundle_status = Vec::with_capacity(self.component_ids.len());

        let current_archetype = &mut archetypes[archetype_id];
        for component_id in self.component_ids.iter().cloned() {
            if current_archetype.contains(component_id) {
                bundle_status.push(ComponentStatus::Mutated);
            } else {
                bundle_status.push(ComponentStatus::Add);
                let component_info = unsafe { components.get_info_unchecked(component_id) };
                match component_info.storage_type() {
                    StorageType::Table => new_table_components.push(component_id),
                    StorageType::SparseSet => new_sparse_set_components.push(component_id),
                }
            }
        }

        if new_table_components.is_empty() && new_sparse_set_components.is_empty() {
            let edges = current_archetype.edges_mut();
            edges.insert_add_bundle(self.id, archetype_id, bundle_status);
            archetype_id
        } else {
            let table_id;
            let table_components;
            let sparse_set_components;
            {
                let current_archetype = &archetypes[archetype_id];
                table_components = if new_table_components.is_empty() {
                    table_id = current_archetype.table_id();
                    current_archetype.table_components().collect()
                } else {
                    new_table_components.extend(current_archetype.table_components());
                    new_table_components.sort();
                    table_id = unsafe {
                        storages
                            .tables
                            .get_id_or_insert(&new_table_components, components)
                    };
                    new_table_components
                };
                sparse_set_components = if new_sparse_set_components.is_empty() {
                    current_archetype.sparse_set_components().collect()
                } else {
                    new_sparse_set_components.extend(current_archetype.sparse_set_components());
                    new_sparse_set_components.sort();
                    new_sparse_set_components
                };
            };
            let new_archetype_id =
                archetypes.get_id_or_insert(table_id, table_components, sparse_set_components);
            archetypes[archetype_id].edges_mut().insert_add_bundle(
                self.id,
                new_archetype_id,
                bundle_status,
            );
            new_archetype_id
        }
    }
}

pub(crate) struct BundleInserter<'a, 'b> {
    pub(crate) archetype: &'a mut Archetype,
    pub(crate) entities: &'a mut Entities,
    bundle_info: &'b BundleInfo,
    table: &'a mut Table,
    sparse_sets: &'a mut SparseSets,
    result: InsertBundleResult<'a>,
    archetypes_ptr: *mut Archetype,
    change_tick: Tick,
}

pub(crate) enum InsertBundleResult<'a> {
    SameArchetype,
    NewArchetypeSameTable {
        new_archetype: &'a mut Archetype,
    },
    NewArchetypeNewTable {
        new_archetype: &'a mut Archetype,
        new_table: &'a mut Table,
    },
}

impl<'a, 'b> BundleInserter<'a, 'b> {
    #[inline]
    pub unsafe fn insert<T: DynamicBundle>(
        &mut self,
        entity: Entity,
        location: EntityLocation,
        bundle: T,
    ) -> EntityLocation {
        match &mut self.result {
            InsertBundleResult::SameArchetype => {
                let add_bundle = unsafe {
                    self.archetype
                        .edges()
                        .get_add_bundle_internal(self.bundle_info.id)
                        .debug_checked_unwrap()
                };
                self.bundle_info.write_components(
                    self.table,
                    self.sparse_sets,
                    add_bundle,
                    entity,
                    location.table_row,
                    self.change_tick,
                    bundle,
                );
                location
            }
            InsertBundleResult::NewArchetypeSameTable { new_archetype } => {
                let result = self.archetype.swap_remove(location.archetype_row);
                if let Some(swapped_entity) = result.swapped_entity {
                    let swapped_location =
                        unsafe { self.entities.get(swapped_entity).debug_checked_unwrap() };
                    self.entities.set(
                        swapped_entity.index(),
                        EntityLocation {
                            archetype_id: swapped_location.archetype_id,
                            archetype_row: location.archetype_row,
                            table_id: swapped_location.table_id,
                            table_row: swapped_location.table_row,
                        },
                    );
                }
                let new_location = new_archetype.allocate(entity, result.table_row);
                self.entities.set(entity.index(), new_location);

                let add_bundle = unsafe {
                    self.archetype
                        .edges()
                        .get_add_bundle_internal(self.bundle_info.id)
                        .debug_checked_unwrap()
                };
                self.bundle_info.write_components(
                    self.table,
                    self.sparse_sets,
                    add_bundle,
                    entity,
                    result.table_row,
                    self.change_tick,
                    bundle,
                );
                new_location
            }
            InsertBundleResult::NewArchetypeNewTable {
                new_archetype,
                new_table,
            } => {
                let result = self.archetype.swap_remove(location.archetype_row);
                if let Some(swapped_entity) = result.swapped_entity {
                    let swapped_location =
                        unsafe { self.entities.get(swapped_entity).debug_checked_unwrap() };
                    self.entities.set(
                        swapped_entity.index(),
                        EntityLocation {
                            archetype_id: swapped_location.archetype_id,
                            archetype_row: location.archetype_row,
                            table_id: swapped_location.table_id,
                            table_row: swapped_location.table_row,
                        },
                    );
                }
                let move_result = self
                    .table
                    .move_to_superset_unchecked(result.table_row, new_table);
                let new_location = new_archetype.allocate(entity, move_result.new_row);
                self.entities.set(entity.index(), new_location);

                if let Some(swapped_entity) = move_result.swapped_entity {
                    let swapped_location =
                        unsafe { self.entities.get(swapped_entity).debug_checked_unwrap() };
                    let swapped_archetype = if self.archetype.id() == swapped_location.archetype_id
                    {
                        &mut *self
                            .archetypes_ptr
                            .add(swapped_location.archetype_id.index())
                    } else if new_archetype.id() == swapped_location.archetype_id {
                        new_archetype
                    } else {
                        &mut *self
                            .archetypes_ptr
                            .add(swapped_location.archetype_id.index())
                    };

                    self.entities.set(
                        swapped_entity.index(),
                        EntityLocation {
                            archetype_id: swapped_location.archetype_id,
                            archetype_row: swapped_location.archetype_row,
                            table_id: swapped_location.table_id,
                            table_row: result.table_row,
                        },
                    );
                    swapped_archetype
                        .set_entity_table_row(swapped_location.archetype_row, result.table_row);
                }

                let add_bundle = unsafe {
                    self.archetype
                        .edges()
                        .get_add_bundle_internal(self.bundle_info.id)
                        .debug_checked_unwrap()
                };
                self.bundle_info.write_components(
                    new_table,
                    self.sparse_sets,
                    add_bundle,
                    entity,
                    move_result.new_row,
                    self.change_tick,
                    bundle,
                );
                new_location
            }
        }
    }
}

pub(crate) struct BundleSpawner<'a, 'b> {
    pub(crate) archetype: &'a mut Archetype,
    pub(crate) entities: &'a mut Entities,
    bundle_info: &'b BundleInfo,
    table: &'a mut Table,
    sparse_sets: &'a mut SparseSets,
    change_tick: Tick,
}

impl<'a, 'b> BundleSpawner<'a, 'b> {
    pub fn reserve_storage(&mut self, additional: usize) {
        self.archetype.reserve(additional);
        self.table.reserve(additional);
    }

    #[inline]
    pub unsafe fn spawn_non_existent<T: DynamicBundle>(
        &mut self,
        entity: Entity,
        bundle: T,
    ) -> EntityLocation {
        let table_row = self.table.allocate(entity);
        let location = self.archetype.allocate(entity, table_row);
        self.bundle_info.write_components(
            self.table,
            self.sparse_sets,
            &SpawnBundleStatus,
            entity,
            table_row,
            self.change_tick,
            bundle,
        );
        self.entities.set(entity.index(), location);
        location
    }

    #[inline]
    pub unsafe fn spawn<T: Bundle>(&mut self, bundle: T) -> Entity {
        let entity = self.entities.alloc();
        self.spawn_non_existent(entity, bundle);
        entity
    }
}

#[derive(Default)]
pub struct Bundles {
    bundle_infos: Vec<BundleInfo>,
    bundle_ids: TypeIdMap<BundleId>,
    dynamic_bundle_ids: HashMap<Vec<ComponentId>, (BundleId, Vec<StorageType>)>,
    dynamic_component_bundle_ids: HashMap<ComponentId, (BundleId, StorageType)>,
}

impl Bundles {
    #[inline]
    pub fn get(&self, bundle_id: BundleId) -> Option<&BundleInfo> {
        self.bundle_infos.get(bundle_id.index())
    }

    #[inline]
    pub fn get_id(&self, type_id: TypeId) -> Option<BundleId> {
        self.bundle_ids.get(&type_id).cloned()
    }

    pub(crate) fn init_info<'a, T: Bundle>(
        &'a mut self,
        components: &mut Components,
        storages: &mut Storages,
    ) -> &'a BundleInfo {
        let bundle_infos = &mut self.bundle_infos;
        let id = self.bundle_ids.entry(TypeId::of::<T>()).or_insert_with(|| {
            let mut component_ids = Vec::new();
            T::component_ids(components, storages, &mut |id| component_ids.push(id));
            let id = BundleId(bundle_infos.len());
            let bundle_info = unsafe {
                BundleInfo::new(std::any::type_name::<T>(), components, component_ids, id)
            };
            bundle_infos.push(bundle_info);
            id
        });
        unsafe { self.bundle_infos.get_unchecked(id.0) }
    }

    pub(crate) fn init_dynamic_info(
        &mut self,
        components: &mut Components,
        comopnent_ids: &[ComponentId],
    ) -> (&BundleInfo, &Vec<StorageType>) {
        let bundle_infos = &mut self.bundle_infos;
        let (_, (bundle_id, storage_types)) = self
            .dynamic_bundle_ids
            .raw_entry_mut()
            .from_key(comopnent_ids)
            .or_insert_with(|| {
                (
                    Vec::from(comopnent_ids),
                    initialize_dynamic_bundle(bundle_infos, components, Vec::from(comopnent_ids)),
                )
            });
        let bundle_info = unsafe { bundle_infos.get_unchecked(bundle_id.0) };
        (bundle_info, storage_types)
    }

    pub(crate) fn init_compoonent_info(
        &mut self,
        components: &mut Components,
        component_id: ComponentId,
    ) -> (&BundleInfo, StorageType) {
        let bundle_infos = &mut self.bundle_infos;
        let (bundle_id, storage_types) = self
            .dynamic_component_bundle_ids
            .entry(component_id)
            .or_insert_with(|| {
                let (id, storage_type) =
                    initialize_dynamic_bundle(bundle_infos, components, vec![component_id]);
                (id, storage_type[0])
            });
        let bundle_info = unsafe { bundle_infos.get_unchecked(bundle_id.0) };
        (bundle_info, *storage_types)
    }
}

fn initialize_dynamic_bundle(
    bundle_infos: &mut Vec<BundleInfo>,
    components: &Components,
    component_ids: Vec<ComponentId>,
) -> (BundleId, Vec<StorageType>) {
    let storage_types = component_ids.iter().map(|&id|{
        components.get_info(id).unwrap_or_else(||{
            panic!("init_dynamic_info called with component id {id:?} which doesn't exist in this world")
        }).storage_type()
    }).collect();

    let id = BundleId(bundle_infos.len());
    let bundle_info = unsafe { BundleInfo::new("<dynamic bundle>", components, component_ids, id) };
    bundle_infos.push(bundle_info);
    (id, storage_types)
}
