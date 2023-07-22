#![allow(unused)]

use core::arch;
use std::ops::{Index, IndexMut};

use joker_foundation::HashMap;

use crate::{
    bundle::BundleId,
    component::{self, ComponentId, StorageType},
    entity::{Entity, EntityLocation},
    storage::{
        ImmutableSparseSet, SparseArray, SparseSet, SparseSetIndex, Table, TableId, TableRow,
    },
};

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(transparent)]
pub struct ArchetypeRow(u32);

impl ArchetypeRow {
    pub const INVALID: ArchetypeRow = ArchetypeRow(u32::MAX);

    #[inline]
    pub const fn new(index: usize) -> Self {
        Self(index as u32)
    }

    #[inline]
    pub const fn index(self) -> usize {
        self.0 as usize
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct ArchetypeId(u32);

impl ArchetypeId {
    pub const EMPTY: ArchetypeId = ArchetypeId(0);
    pub const INVALID: ArchetypeId = ArchetypeId(u32::MAX);

    #[inline]
    pub(crate) const fn new(index: usize) -> Self {
        Self(index as u32)
    }

    #[inline]
    pub(crate) const fn index(self) -> usize {
        self.0 as usize
    }
}

#[derive(Copy, Clone)]
pub enum ComponentStatus {
    Add,
    Mutated,
}

pub struct AddBundle {
    pub archetype_id: ArchetypeId,
    pub bundle_status: Vec<ComponentStatus>,
}

pub trait BundleComponentStatus {
    ///调用者必须保证 index 是合法的
    unsafe fn get_status(&self, index: usize) -> ComponentStatus;
}

impl BundleComponentStatus for AddBundle {
    #[inline]
    unsafe fn get_status(&self, index: usize) -> ComponentStatus {
        *self.bundle_status.get_unchecked(index)
    }
}

pub(crate) struct SpawnBundleStatus;

impl BundleComponentStatus for SpawnBundleStatus {
    #[inline]
    unsafe fn get_status(&self, index: usize) -> ComponentStatus {
        ComponentStatus::Add
    }
}

#[derive(Default)]
pub struct Edges {
    add_bundle: SparseArray<BundleId, AddBundle>,
    remove_bundle: SparseArray<BundleId, Option<ArchetypeId>>,
    take_bundle: SparseArray<BundleId, Option<ArchetypeId>>,
}

impl Edges {
    #[inline]
    pub fn get_add_bundle(&self, bundle_id: BundleId) -> Option<ArchetypeId> {
        self.get_add_bundle_internal(bundle_id)
            .map(|bundle| bundle.archetype_id)
    }

    #[inline]
    pub(crate) fn get_add_bundle_internal(&self, bundle_id: BundleId) -> Option<&AddBundle> {
        self.add_bundle.get(bundle_id)
    }

    #[inline]
    pub(crate) fn insert_add_bundle(
        &mut self,
        bundle_id: BundleId,
        archetype_id: ArchetypeId,
        bundle_status: Vec<ComponentStatus>,
    ) {
        self.add_bundle.insert(
            bundle_id,
            AddBundle {
                archetype_id,
                bundle_status,
            },
        )
    }

    #[inline]
    pub fn get_remove_bundle(&self, bundle_id: BundleId) -> Option<Option<ArchetypeId>> {
        self.remove_bundle.get(bundle_id).cloned()
    }

    #[inline]
    pub(crate) fn insert_remove_bundle(
        &mut self,
        bundle_id: BundleId,
        archetype_id: Option<ArchetypeId>,
    ) {
        self.remove_bundle.insert(bundle_id, archetype_id);
    }

    #[inline]
    pub fn get_take_bundle(&self, bundle_id: BundleId) -> Option<Option<ArchetypeId>> {
        self.take_bundle.get(bundle_id).cloned()
    }

    #[inline]
    pub(crate) fn insert_take_bundle(
        &mut self,
        bundle_id: BundleId,
        archetype_id: Option<ArchetypeId>,
    ) {
        self.take_bundle.insert(bundle_id, archetype_id);
    }
}

pub struct ArchetypeEntity {
    entity: Entity,
    table_row: TableRow,
}

impl ArchetypeEntity {
    #[inline]
    pub const fn entity(&self) -> Entity {
        self.entity
    }

    #[inline]
    pub const fn table_row(&self) -> TableRow {
        self.table_row
    }
}

pub(crate) struct ArchetypeSwapRemoveResult {
    pub(crate) swapped_entity: Option<Entity>,
    pub(crate) table_row: TableRow,
}

struct ArchetypeComponentInfo {
    storage_type: StorageType,
    archetype_component_id: ArchetypeComponentId,
}

pub struct Archetype {
    id: ArchetypeId,
    table_id: TableId,
    edges: Edges,
    entities: Vec<ArchetypeEntity>,
    components: ImmutableSparseSet<ComponentId, ArchetypeComponentInfo>,
}

impl Archetype {
    pub(crate) fn new(
        id: ArchetypeId,
        table_id: TableId,
        table_components: impl Iterator<Item = (ComponentId, ArchetypeComponentId)>,
        sparse_set_components: impl Iterator<Item = (ComponentId, ArchetypeComponentId)>,
    ) -> Self {
        let (min_table, _) = table_components.size_hint();
        let (min_sparse, _) = sparse_set_components.size_hint();
        let mut components = SparseSet::with_capacity(min_table + min_sparse);
        for (component_id, archetype_component_id) in table_components {
            components.insert(
                component_id,
                ArchetypeComponentInfo {
                    storage_type: StorageType::Table,
                    archetype_component_id,
                },
            );
        }

        for (component_id, archetype_component_id) in sparse_set_components {
            components.insert(
                component_id,
                ArchetypeComponentInfo {
                    storage_type: StorageType::SparseSet,
                    archetype_component_id,
                },
            );
        }

        Self {
            id,
            table_id,
            edges: Default::default(),
            entities: Vec::new(),
            components: components.into_immutable(),
        }
    }

    #[inline]
    pub fn id(&self) -> ArchetypeId {
        self.id
    }

    #[inline]
    pub fn table_id(&self) -> TableId {
        self.table_id
    }

    #[inline]
    pub fn entities(&self) -> &[ArchetypeEntity] {
        &self.entities
    }

    #[inline]
    pub fn table_components(&self) -> impl Iterator<Item = ComponentId> + '_ {
        self.components
            .iter()
            .filter(|(_, component)| component.storage_type == StorageType::Table)
            .map(|(id, _)| *id)
    }

    #[inline]
    pub fn sparse_set_components(&self) -> impl Iterator<Item = ComponentId> + '_ {
        self.components
            .iter()
            .filter(|(_, component)| component.storage_type == StorageType::SparseSet)
            .map(|(id, _)| *id)
    }

    #[inline]
    pub fn components(&self) -> impl Iterator<Item = ComponentId> + '_ {
        self.components.indices()
    }

    #[inline]
    pub fn edges(&self) -> &Edges {
        &self.edges
    }

    #[inline]
    pub(crate) fn edges_mut(&mut self) -> &mut Edges {
        &mut self.edges
    }

    #[inline]
    pub fn entity_table_row(&self, row: ArchetypeRow) -> TableRow {
        self.entities[row.index()].table_row
    }

    #[inline]
    pub(crate) fn set_entity_table_row(&mut self, row: ArchetypeRow, table_row: TableRow) {
        self.entities[row.index()].table_row = table_row
    }

    #[inline]
    pub(crate) unsafe fn allocate(
        &mut self,
        entity: Entity,
        table_row: TableRow,
    ) -> EntityLocation {
        let archetype_row = ArchetypeRow::new(self.entities.len());
        self.entities.push(ArchetypeEntity { entity, table_row });
        EntityLocation {
            archetype_id: self.id,
            archetype_row,
            table_id: self.table_id,
            table_row,
        }
    }

    #[inline]
    pub(crate) fn reserve(&mut self, additional: usize) {
        self.entities.reserve(additional)
    }

    #[inline]
    pub(crate) fn swap_remove(&mut self, row: ArchetypeRow) -> ArchetypeSwapRemoveResult {
        let is_last = row.index() == self.entities.len() - 1;
        let entity = self.entities.swap_remove(row.index());
        ArchetypeSwapRemoveResult {
            swapped_entity: if is_last {
                None
            } else {
                Some(self.entities[row.index()].entity)
            },
            table_row: entity.table_row,
        }
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.entities.len()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.entities.is_empty()
    }

    #[inline]
    pub fn contains(&self, component_id: ComponentId) -> bool {
        self.components.contains(component_id)
    }

    #[inline]
    pub fn get_storage_type(&self, component_id: ComponentId) -> Option<StorageType> {
        self.components
            .get(component_id)
            .map(|info| info.storage_type)
    }

    #[inline]
    pub fn get_archetype_component_id(
        &self,
        component_id: ComponentId,
    ) -> Option<ArchetypeComponentId> {
        self.components
            .get(component_id)
            .map(|info| info.archetype_component_id)
    }

    pub(crate) fn clear_entities(&mut self) {
        self.entities.clear();
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Ord, PartialOrd)]
pub struct ArchetypeGeneration(usize);

impl ArchetypeGeneration {
    #[inline]
    pub(crate) const fn initial() -> Self {
        ArchetypeGeneration(0)
    }

    #[inline]
    pub(crate) fn value(self) -> usize {
        self.0
    }
}

#[derive(Hash, PartialEq, Eq)]
struct ArchetypeIdentity {
    table_components: Box<[ComponentId]>,
    sparse_set_components: Box<[ComponentId]>,
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
pub struct ArchetypeComponentId(usize);

impl SparseSetIndex for ArchetypeComponentId {
    #[inline]
    fn sparse_set_index(&self) -> usize {
        self.0
    }

    fn get_sparse_set_index(value: usize) -> Self {
        Self(value)
    }
}

pub struct Archetypes {
    pub(crate) archetypes: Vec<Archetype>,
    pub(crate) archetype_component_count: usize,
    archetype_ids: HashMap<ArchetypeIdentity, ArchetypeId>,
}

impl Archetypes {
    pub(crate) fn new() -> Self {
        let mut archetypes = Archetypes {
            archetypes: Vec::new(),
            archetype_component_count: 0,
            archetype_ids: Default::default(),
        };
        archetypes.get_id_or_insert(TableId::empty(), Vec::new(), Vec::new());
        archetypes
    }

    #[inline]
    pub fn generation(&self) -> ArchetypeGeneration {
        ArchetypeGeneration(self.archetypes.len())
    }

    #[inline]
    #[allow(clippy::len_without_is_empty)]
    pub fn len(&self) -> usize {
        self.archetypes.len()
    }

    #[inline]
    pub fn empty(&self) -> &Archetype {
        unsafe { self.archetypes.get_unchecked(ArchetypeId::EMPTY.index()) }
    }

    #[inline]
    pub(crate) fn empty_mut(&mut self) -> &mut Archetype {
        unsafe {
            self.archetypes
                .get_unchecked_mut(ArchetypeId::EMPTY.index())
        }
    }

    #[inline]
    pub fn get(&self, id: ArchetypeId) -> Option<&Archetype> {
        self.archetypes.get(id.index())
    }

    #[inline]
    pub(crate) fn get_2_mut(
        &mut self,
        a: ArchetypeId,
        b: ArchetypeId,
    ) -> (&mut Archetype, &mut Archetype) {
        if a.index() > b.index() {
            let (b_slice, a_slice) = self.archetypes.split_at_mut(a.index());
            (&mut a_slice[0], &mut b_slice[b.index()])
        } else {
            let (a_slice, b_slice) = self.archetypes.split_at_mut(b.index());
            (&mut a_slice[a.index()], &mut b_slice[0])
        }
    }

    #[inline]
    pub fn iter(&self) -> impl Iterator<Item = &Archetype> {
        self.archetypes.iter()
    }

    pub(crate) fn get_id_or_insert(
        &mut self,
        table_id: TableId,
        table_components: Vec<ComponentId>,
        sparse_set_components: Vec<ComponentId>,
    ) -> ArchetypeId {
        let archetype_identity = ArchetypeIdentity {
            sparse_set_components: sparse_set_components.clone().into_boxed_slice(),
            table_components: table_components.clone().into_boxed_slice(),
        };

        let archetypes = &mut self.archetypes;
        let archetype_component_count = &mut self.archetype_component_count;
        *self
            .archetype_ids
            .entry(archetype_identity)
            .or_insert_with(move || {
                let id = ArchetypeId::new(archetypes.len());
                let table_start = *archetype_component_count;
                *archetype_component_count += table_components.len();
                let table_archetype_components =
                    (table_start..*archetype_component_count).map(ArchetypeComponentId);
                let sparse_start = *archetype_component_count;
                *archetype_component_count += sparse_set_components.len();
                let sparse_set_archetype_components =
                    (sparse_start..*archetype_component_count).map(ArchetypeComponentId);
                archetypes.push(Archetype::new(
                    id,
                    table_id,
                    table_components.into_iter().zip(table_archetype_components),
                    sparse_set_components
                        .into_iter()
                        .zip(sparse_set_archetype_components),
                ));
                id
            })
    }

    #[inline]
    pub fn archetype_components_len(&self) -> usize {
        self.archetype_component_count
    }

    pub(crate) fn clear_entities(&mut self) {
        for archetype in &mut self.archetypes {
            archetype.clear_entities();
        }
    }
}

impl Index<ArchetypeId> for Archetypes {
    type Output = Archetype;

    #[inline]
    fn index(&self, index: ArchetypeId) -> &Self::Output {
        &self.archetypes[index.index()]
    }
}

impl IndexMut<ArchetypeId> for Archetypes {
    #[inline]
    fn index_mut(&mut self, index: ArchetypeId) -> &mut Self::Output {
        &mut self.archetypes[index.index()]
    }
}
