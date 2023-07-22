#![allow(unused)]

use joker_foundation::HashMap;
use joker_ptr::{OwningPtr, Ptr, PtrMut, UnsafeCellDeref};

use super::{blob_vec::BlobVec, ImmutableSparseArray, ImmutableSparseSet, SparseSet};
use crate::{
    component::{
        self, ComponentId, ComponentInfo, ComponentTicks, Components, Tick,
        TickCells,
    },
    entity::Entity,
    query::DebugCheckedUnwrap,
};
use std::{alloc::Layout, cell::UnsafeCell, ops::{Index, IndexMut}};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableId(u32);

impl TableId {
    pub const INVALID: TableId = TableId(u32::MAX);

    #[inline]
    pub fn new(index: usize) -> Self {
        TableId(index as u32)
    }

    #[inline]
    pub fn index(&self) -> usize {
        self.0 as usize
    }

    #[inline]
    pub fn empty() -> Self {
        TableId(0)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(transparent)]
pub struct TableRow(u32);

impl TableRow {
    pub const INVALID: TableRow = TableRow(u32::MAX);

    #[inline]
    pub const fn new(index: usize) -> Self {
        Self(index as u32)
    }

    #[inline]
    pub const fn index(&self) -> usize {
        self.0 as usize
    }
}

#[derive(Debug)]
pub struct Column {
    data: BlobVec,
    added_ticks: Vec<UnsafeCell<Tick>>,
    changed_ticks: Vec<UnsafeCell<Tick>>,
}

impl Column {
    #[inline]
    pub fn with_capacity(component_info: &ComponentInfo, capacity: usize) -> Self {
        Self {
            data: unsafe { BlobVec::new(component_info.layout(), component_info.drop(), capacity) },
            added_ticks: Vec::with_capacity(capacity),
            changed_ticks: Vec::with_capacity(capacity),
        }
    }

    #[inline]
    pub fn item_layout(&self) -> Layout {
        self.data.layout()
    }

    #[inline]
    pub unsafe fn initialize(&mut self, row: TableRow, data: OwningPtr<'_>, tick: Tick) {
        debug_assert!(row.index() < self.len());
        self.data.initialize_unchecked(row.index(), data);
        *self.added_ticks.get_unchecked_mut(row.index()).get_mut() = tick;
        *self.changed_ticks.get_unchecked_mut(row.index()).get_mut() = tick;
    }

    #[inline]
    pub unsafe fn replace(&mut self, row: TableRow, data: OwningPtr<'_>, change_tick: Tick) {
        debug_assert!(row.index() < self.len());
        self.data.replace_unchecked(row.index(), data);
        *self.changed_ticks.get_unchecked_mut(row.index()).get_mut() = change_tick;
    }

    #[inline]
    pub unsafe fn replace_untracked(&mut self, row: TableRow, data: OwningPtr<'_>) {
        debug_assert!(row.index() < self.len());
        self.data.replace_unchecked(row.index(), data);
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.data.len()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    #[inline]
    pub unsafe fn swap_remove_unchecked(&mut self, row: TableRow) {
        self.data.swap_remove_and_drop_unchecked(row.index());
        self.added_ticks.swap_remove(row.index());
        self.changed_ticks.swap_remove(row.index());
    }

    #[inline]
    #[must_use = "返回值必须被使用，要不内存就丢了"]
    pub(crate) fn swap_remove_and_forget(
        &mut self,
        row: TableRow,
    ) -> Option<(OwningPtr<'_>, ComponentTicks)> {
        (row.index() < self.data.len()).then(|| {
            let data = unsafe { self.data.swap_remove_and_forget_unchecked(row.index()) };
            let added = self.added_ticks.swap_remove(row.index()).into_inner();
            let changed = self.changed_ticks.swap_remove(row.index()).into_inner();
            (data, ComponentTicks { added, changed })
        })
    }

    #[inline]
    #[must_use = "返回值必须被使用，否则内存就丢了"]
    pub(crate) unsafe fn swap_remove_and_forget_unchecked(
        &mut self,
        row: TableRow,
    ) -> (OwningPtr<'_>, ComponentTicks) {
        let data = self.data.swap_remove_and_forget_unchecked(row.index());
        let added = self.added_ticks.swap_remove(row.index()).into_inner();
        let changed = self.changed_ticks.swap_remove(row.index()).into_inner();
        (data, ComponentTicks { added, changed })
    }

    #[inline]
    pub(crate) unsafe fn initialize_from_unchecked(
        &mut self,
        other: &mut Column,
        src_row: TableRow,
        dst_row: TableRow,
    ) {
        debug_assert!(self.data.layout() == other.data.layout());
        let ptr = self.data.get_unchecked_mut(dst_row.index());
        other.data.swap_remove_unchecked(src_row.index(), ptr);
        *self.added_ticks.get_unchecked_mut(dst_row.index()) =
            other.added_ticks.swap_remove(src_row.index());
        *self.changed_ticks.get_unchecked_mut(dst_row.index()) =
            other.changed_ticks.swap_remove(src_row.index());
    }

    pub(crate) unsafe fn push(&mut self, ptr: OwningPtr<'_>, ticks: ComponentTicks) {
        self.data.push(ptr);
        self.added_ticks.push(UnsafeCell::new(ticks.added));
        self.changed_ticks.push(UnsafeCell::new(ticks.changed));
    }

    #[inline]
    pub(crate) fn reserve_exact(&mut self, additional: usize) {
        self.data.reserve_exact(additional);
        self.added_ticks.reserve_exact(additional);
        self.changed_ticks.reserve_exact(additional);
    }

    #[inline]
    pub fn get_data_ptr(&self) -> Ptr<'_> {
        self.data.get_ptr()
    }

    pub unsafe fn get_dat_slice<T>(&self) -> &[UnsafeCell<T>] {
        self.data.get_slice()
    }

    #[inline]
    pub fn get_added_ticks_slice(&self) -> &[UnsafeCell<Tick>] {
        return &self.added_ticks;
    }

    #[inline]
    pub fn get_changed_ticks_slice(&self) -> &[UnsafeCell<Tick>] {
        return &self.changed_ticks;
    }

    #[inline]
    pub fn get(&self, row: TableRow) -> Option<(Ptr<'_>, TickCells<'_>)> {
        (row.index() < self.data.len()).then(|| unsafe {
            (
                self.data.get_unchecked(row.index()),
                TickCells {
                    added: self.added_ticks.get_unchecked(row.index()),
                    changed: self.changed_ticks.get_unchecked(row.index()),
                },
            )
        })
    }

    #[inline]
    pub fn get_data(&self, row: TableRow) -> Option<Ptr<'_>> {
        (row.index() < self.data.len()).then(|| unsafe { self.data.get_unchecked(row.index()) })
    }

    #[inline]
    pub unsafe fn get_data_unchecked(&self, row: TableRow) -> Ptr<'_> {
        debug_assert!(row.index() < self.data.len());
        self.data.get_unchecked(row.index())
    }

    #[inline]
    pub fn get_data_mut(&mut self, row: TableRow) -> Option<PtrMut<'_>> {
        (row.index() < self.data.len()).then(|| unsafe { self.data.get_unchecked_mut(row.index()) })
    }

    #[inline]
    pub(crate) unsafe fn get_data_unchecked_mut(&self, row: TableRow) -> PtrMut<'_> {
        debug_assert!(row.index() < self.data.len());
        self.data.get_unchecked_mut(row.index())
    }

    #[inline]
    pub fn get_added_ticks(&self, row: TableRow) -> Option<&UnsafeCell<Tick>> {
        self.added_ticks.get(row.index())
    }
    #[inline]
    pub fn get_changed_ticks(&self, row: TableRow) -> Option<&UnsafeCell<Tick>> {
        self.changed_ticks.get(row.index())
    }

    #[inline]
    pub fn get_ticks(&self, row: TableRow) -> Option<ComponentTicks> {
        if row.index() < self.data.len() {
            Some(unsafe { self.get_ticks_unchecked(row) })
        } else {
            None
        }
    }

    #[inline]
    pub unsafe fn get_added_ticks_unchecked(&self, row: TableRow) -> &UnsafeCell<Tick> {
        debug_assert!(row.index() < self.added_ticks.len());
        self.added_ticks.get_unchecked(row.index())
    }

    #[inline]
    pub unsafe fn get_changed_ticks_unchecked(&self, row: TableRow) -> &UnsafeCell<Tick> {
        debug_assert!(row.index() < self.changed_ticks.len());
        self.changed_ticks.get_unchecked(row.index())
    }

    #[inline]
    pub unsafe fn get_ticks_unchecked(&self, row: TableRow) -> ComponentTicks {
        debug_assert!(row.index() < self.added_ticks.len());
        debug_assert!(row.index() < self.changed_ticks.len());
        ComponentTicks {
            added: self.added_ticks.get_unchecked(row.index()).read(),
            changed: self.changed_ticks.get_unchecked(row.index()).read(),
        }
    }

    pub fn clear(&mut self) {
        self.data.clear();
        self.added_ticks.clear();
        self.changed_ticks.clear();
    }

    #[inline]
    pub(crate) fn check_change_ticks(&mut self, change_tick: Tick) {
        for component_ticks in &mut self.added_ticks {
            component_ticks.get_mut().check_tick(change_tick);
        }
        for component_ticks in &mut self.changed_ticks {
            component_ticks.get_mut().check_tick(change_tick);
        }
    }
}

pub(crate) struct TableBuilder {
    columns: SparseSet<ComponentId, Column>,
    capacity: usize,
}

impl TableBuilder {
    pub fn with_capacity(capacity: usize, column_capacity: usize) -> Self {
        Self {
            columns: SparseSet::with_capacity(column_capacity),
            capacity: capacity,
        }
    }

    pub fn add_column(&mut self, component_info: &ComponentInfo) {
        self.columns.insert(
            component_info.id(),
            Column::with_capacity(component_info, self.capacity),
        )
    }

    pub fn build(self) -> Table {
        Table {
            columns: self.columns.into_immutable(),
            entities: Vec::with_capacity(self.capacity),
        }
    }
}

pub struct Table {
    columns: ImmutableSparseSet<ComponentId, Column>,
    entities: Vec<Entity>,
}

impl Table {
    #[inline]
    pub fn entities(&self) -> &[Entity] {
        &self.entities
    }

    pub(crate) unsafe fn swap_remove_unchecked(&mut self, row: TableRow) -> Option<Entity> {
        for column in self.columns.values_mut() {
            column.swap_remove_unchecked(row);
        }
        let is_last = row.index() == self.entities.len() - 1;
        self.entities.swap_remove(row.index());
        if is_last {
            None
        } else {
            Some(self.entities[row.index()])
        }
    }

    pub(crate) unsafe fn move_to_and_forget_missing_unchecked(
        &mut self,
        row: TableRow,
        new_table: &mut Table,
    ) -> TableMoveResult {
        debug_assert!(row.index() == self.entity_count());
        let is_last = row.index() == self.entities.len() - 1;
        let new_row = new_table.allocate(self.entities.swap_remove(row.index()));
        for (component_id, column) in self.columns.iter_mut() {
            if let Some(new_column) = new_table.get_column_mut(*component_id) {
                new_column.initialize_from_unchecked(column, row, new_row);
            } else {
                let (_, _) = column.swap_remove_and_forget_unchecked(row);
            }
        }
        TableMoveResult {
            new_row,
            swapped_entity: if is_last {
                None
            } else {
                Some(self.entities[row.index()])
            },
        }
    }

    pub(crate) unsafe fn move_to_and_drop_missing_unchecked(
        &mut self,
        row: TableRow,
        new_table: &mut Table,
    ) -> TableMoveResult {
        debug_assert!(row.index() == self.entity_count());
        let is_last = row.index() == self.entities.len() - 1;
        let new_row = new_table.allocate(self.entities.swap_remove(row.index()));
        for (component_id, column) in self.columns.iter_mut() {
            if let Some(new_column) = new_table.get_column_mut(*component_id) {
                new_column.initialize_from_unchecked(column, row, new_row);
            } else {
                column.swap_remove_unchecked(row);
            }
        }
        TableMoveResult {
            new_row,
            swapped_entity: if is_last {
                None
            } else {
                Some(self.entities[row.index()])
            },
        }
    }

    pub(crate) unsafe fn move_to_superset_unchecked(
        &mut self,
        row: TableRow,
        new_table: &mut Table,
    ) -> TableMoveResult {
        debug_assert!(row.index() == self.entity_count());
        let is_last = row.index() == self.entities.len() - 1;
        let new_row = new_table.allocate(self.entities.swap_remove(row.index()));
        for (component_id, column) in self.columns.iter_mut() {
            new_table
                .get_column_mut(*component_id)
                .debug_checked_unwrap()
                .initialize_from_unchecked(column, row, new_row);
        }
        TableMoveResult {
            new_row,
            swapped_entity: if is_last {
                None
            } else {
                Some(self.entities[row.index()])
            },
        }
    }

    #[inline]
    pub fn get_column(&self, component_id: ComponentId) -> Option<&Column> {
        self.columns.get(component_id)
    }

    #[inline]
    pub(crate) fn get_column_mut(&mut self, component_id: ComponentId) -> Option<&mut Column> {
        self.columns.get_mut(component_id)
    }

    #[inline]
    pub(crate) fn has_column(&self, component_id: ComponentId) -> bool {
        self.columns.contains(component_id)
    }

    pub(crate) fn reserve(&mut self, additional: usize) {
        if self.entities.capacity() - self.entities.len() < additional {
            self.entities.reserve(additional);

            let new_capacity = self.entities.capacity();

            for column in self.columns.values_mut() {
                column.reserve_exact(new_capacity - column.len());
            }
        }
    }

    pub(crate) unsafe fn allocate(&mut self, entity: Entity) -> TableRow {
        self.reserve(1);
        let index = self.entities.len();
        self.entities.push(entity);
        for column in self.columns.values_mut() {
            column.data.set_len(self.entities.len());
            column.added_ticks.push(UnsafeCell::new(Tick::new(0)));
            column.changed_ticks.push(UnsafeCell::new(Tick::new(0)));
        }
        TableRow::new(index)
    }

    #[inline]
    pub fn entity_count(&self) -> usize {
        self.entities.len()
    }

    #[inline]
    pub fn component_count(&self) -> usize {
        self.columns.len()
    }

    #[inline]
    pub fn entity_capacity(&self) -> usize {
        self.entities.capacity()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.entities.is_empty()
    }

    pub(crate) fn check_change_ticks(&mut self, change_tick: Tick) {
        for column in self.columns.values_mut() {
            column.check_change_ticks(change_tick);
        }
    }

    pub fn iter(&self) -> impl Iterator<Item = &Column> {
        self.columns.values()
    }

    pub(crate) fn clear(&mut self) {
        self.entities.clear();
        for column in self.columns.values_mut() {
            column.clear();
        }
    }
}

pub struct Tables {
    tables: Vec<Table>,
    table_ids: HashMap<Vec<ComponentId>, TableId>,
}

impl Default for Tables {
    fn default() -> Self {
        let empty_table = TableBuilder::with_capacity(0, 0).build();
        Tables {
            tables: vec![empty_table],
            table_ids: HashMap::default(),
        }
    }
}

pub(crate) struct TableMoveResult {
    pub swapped_entity: Option<Entity>,
    pub new_row: TableRow,
}

impl Tables {
    #[inline]
    pub fn len(&self) -> usize {
        self.tables.len()
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.tables.is_empty()
    }

    #[inline]
    pub fn get(&self, id: TableId) -> Option<&Table> {
        self.tables.get(id.index())
    }

    #[inline]
    pub(crate) fn get_2_mut(&mut self, a: TableId, b: TableId) -> (&mut Table, &mut Table) {
        if a.index() > b.index() {
            let (b_slice, a_slice) = self.tables.split_at_mut(a.index());
            (&mut a_slice[0], &mut b_slice[b.index()])
        } else {
            let (a_slice, b_slice) = self.tables.split_at_mut(b.index());
            (&mut a_slice[a.index()], &mut b_slice[0])
        }
    }

    pub(crate) unsafe fn get_id_or_insert(
        &mut self,
        component_ids: &[ComponentId],
        components: &Components,
    ) -> TableId {
        let tables = &mut self.tables;
        let (_key, value) = self
            .table_ids
            .raw_entry_mut()
            .from_key(component_ids)
            .or_insert_with(|| {
                let mut table = TableBuilder::with_capacity(0, component_ids.len());
                for component_id in component_ids {
                    table.add_column(components.get_info_unchecked(*component_id));
                }
                tables.push(table.build());
                (component_ids.to_vec(), TableId::new(tables.len() - 1))
            });
        *value
    }

    pub fn iter(&self)->std::slice::Iter<'_,Table>{
        self.tables.iter()
    }

    pub(crate) fn clear(&mut self){
        for table in &mut self.tables{
            table.clear()
        }
    }

    pub(crate) fn check_change_ticks(&mut self, change_tick:Tick){
        for table in &mut self.tables{
            table.check_change_ticks(change_tick);
        }
    }
}

impl Index<TableId> for Tables{
    type Output = Table;
    
    #[inline]
    fn index(&self, index: TableId) -> &Self::Output {
        &self.tables[index.index()]
    }
}

impl IndexMut<TableId> for Tables{
    #[inline]
    fn index_mut(&mut self, index: TableId) -> &mut Self::Output {
        &mut self.tables[index.index()]
    }
}

#[cfg(test)]
mod tests{
    // TODO
}