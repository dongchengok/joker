use std::{mem::ManuallyDrop, thread::ThreadId};

use joker_ptr::{OwningPtr, Ptr, UnsafeCellDeref};

use crate::{
    archetype::ArchetypeComponentId,
    change_detection::{MutUntyped, TicksMut},
    component::{ComponentId, ComponentTicks, Components, Tick, TickCells},
};

use super::{
    table::{Column, TableRow},
    SparseSet,
};

pub struct ResourceData<const SEND: bool> {
    column: ManuallyDrop<Column>,
    type_name: String,
    id: ArchetypeComponentId,
    origin_thread_id: Option<ThreadId>,
}

impl<const SEND: bool> Drop for ResourceData<SEND> {
    fn drop(&mut self) {
        if self.is_present() {
            if std::thread::panicking() {
                return;
            }
            self.validate_access();
        }
        unsafe {
            ManuallyDrop::drop(&mut self.column);
        }
    }
}

impl<const SEND: bool> ResourceData<SEND> {
    const ROW: TableRow = TableRow::new(0);

    #[inline]
    fn validate_access(&self) {
        if SEND {
            return;
        }
        if self.origin_thread_id != Some(std::thread::current().id()) {
            panic!(
                "Attempted to access or drop non-send resource {} from thread {:?} on a thread {:?}. This is not allowed. Aborting.",
                self.type_name,
                self.origin_thread_id,
                std::thread::current().id()
            );
        }
    }

    #[inline]
    pub fn is_present(&self) -> bool {
        !self.column.is_empty()
    }

    #[inline]
    pub fn id(&self) -> ArchetypeComponentId {
        self.id
    }

    #[inline]
    pub fn get_data(&self) -> Option<Ptr<'_>> {
        self.column.get_data(Self::ROW).map(|res| {
            self.validate_access();
            res
        })
    }

    #[inline]
    pub fn get_ticks(&self) -> Option<ComponentTicks> {
        self.column.get_ticks(Self::ROW)
    }

    #[inline]
    pub(crate) fn get_with_ticks(&self) -> Option<(Ptr<'_>, TickCells<'_>)> {
        self.column.get(Self::ROW).map(|res| {
            self.validate_access();
            res
        })
    }

    pub(crate) fn get_mut(&mut self, last_run: Tick, this_run: Tick) -> Option<MutUntyped<'_>> {
        let (ptr, ticks) = self.get_with_ticks()?;
        Some(MutUntyped {
            value: unsafe { ptr.assert_unique() },
            ticks: unsafe { TicksMut::from_tick_cells(ticks, last_run, this_run) },
        })
    }

    #[inline]
    pub(crate) unsafe fn insert(&mut self, value: OwningPtr<'_>, change_tick: Tick) {
        if self.is_present() {
            self.validate_access();
            self.column.replace(Self::ROW, value, change_tick);
        } else {
            if !SEND {
                self.origin_thread_id = Some(std::thread::current().id());
            }
            self.column.push(value, ComponentTicks::new(change_tick));
        }
    }

    #[inline]
    pub(crate) unsafe fn insert_with_ticks(
        &mut self,
        value: OwningPtr<'_>,
        change_ticks: ComponentTicks,
    ) {
        if self.is_present() {
            self.validate_access();
            self.column.replace_untracked(Self::ROW, value);
            *self.column.get_added_ticks_unchecked(Self::ROW).deref_mut() = change_ticks.added;
            *self
                .column
                .get_changed_ticks_unchecked(Self::ROW)
                .deref_mut() = change_ticks.changed;
        } else {
            if !SEND {
                self.origin_thread_id = Some(std::thread::current().id());
            }
            self.column.push(value, change_ticks);
        }
    }

    #[inline]
    #[must_use = "返回的指针需要被使用或者删除"]
    pub(crate) fn remove(&mut self) -> Option<(OwningPtr<'_>, ComponentTicks)> {
        if SEND {
            self.column.swap_remove_and_forget(Self::ROW)
        } else {
            self.is_present()
                .then(|| self.validate_access())
                .and_then(|_| self.column.swap_remove_and_forget(Self::ROW))
        }
    }

    #[inline]
    pub(crate) fn remove_and_drop(&mut self) {
        if self.is_present() {
            self.validate_access();
            self.column.clear();
        }
    }
}

#[derive(Default)]
pub struct Resources<const SEND: bool> {
    resources: SparseSet<ComponentId, ResourceData<SEND>>,
}

impl<const SEND: bool> Resources<SEND> {
    #[inline]
    pub fn len(&self) -> usize {
        self.resources.len()
    }

    pub fn iter(&self) -> impl Iterator<Item = (ComponentId, &ResourceData<SEND>)> {
        self.resources.iter().map(|(id, data)| (*id, data))
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.resources.is_empty()
    }

    #[inline]
    pub fn get(&self, component_id: ComponentId) -> Option<&ResourceData<SEND>> {
        self.resources.get(component_id)
    }

    #[inline]
    pub fn clear(&mut self) {
        self.resources.clear()
    }

    #[inline]
    pub(crate) fn get_mut(&mut self, component_id: ComponentId) -> Option<&mut ResourceData<SEND>> {
        self.resources.get_mut(component_id)
    }

    pub(crate) fn initialize_with(
        &mut self,
        component_id: ComponentId,
        components: &Components,
        f: impl FnOnce() -> ArchetypeComponentId,
    ) -> &mut ResourceData<SEND> {
        self.resources.get_or_insert_with(component_id, ||{
            let component_info = components.get_info(component_id).unwrap();
            if SEND{
                assert!(
                    component_info.is_send_and_sync(),
                    "Send + Sync resource {} initialized as non_send. It may have been inserted via World::insert_non_send_resource by accident. Try using World::insert_resource instead.",
                    component_info.name(),
                );
            }
            ResourceData{
                column:ManuallyDrop::new(Column::with_capacity(component_info, 1)),
                type_name:String::from(component_info.name()),
                id:f(),
                origin_thread_id:None,
            }
        })
    }

    pub(crate) fn check_change_ticks(&mut self, change_tick: Tick) {
        for info in self.resources.values_mut() {
            info.column.check_change_ticks(change_tick);
        }
    }
}
