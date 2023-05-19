#![allow(unused)]

use joker_ptr::UnsafeCellDeref;
use std::ops::{Deref, DerefMut};

use crate::{
    component::{Tick, TickCells},
    system::system_param::Resource,
};

// 这里还没搞明白为什么，tick的检查必须在system没有执行的时候做，所以如果需要检查N次，World就tick了N-1次
// (518,400,000 = 1000 ticks per frame * 144 frames per second * 3600 seconds per hour)
pub const CHECK_TICK_THRESHOLD: u32 = 518_400_000;
// 这个变量目前完全没搞定用途
pub const MAX_CHANGE_AGE: u32 = u32::MAX - (2 * CHECK_TICK_THRESHOLD - 1);

#[derive(Clone)]
pub struct Ticks<'a> {
    pub added: &'a Tick,
    pub changed: &'a Tick,
    pub last_run: Tick,
    pub this_run: Tick,
}

pub struct TicksMut<'a> {
    pub added: &'a mut Tick,
    pub changed: &'a mut Tick,
    pub last_run: Tick,
    pub this_run: Tick,
}

pub struct Mut<'a, T: ?Sized> {
    pub value: &'a mut T,
    pub ticks: TicksMut<'a>,
}

pub struct NonSendMut<'a, T: ?Sized + 'static> {
    pub value: &'a mut T,
    pub ticks: TicksMut<'a>,
}

pub trait DetectChanges {
    fn is_added(&self) -> bool;
    fn is_changed(&self) -> bool;
    fn last_changed(&self) -> Tick;
}

pub struct Res<'a, T: ?Sized + Resource> {
    pub value: &'a T,
    pub ticks: Ticks<'a>,
}

pub struct ResMut<'a, T: ?Sized + Resource> {
    pub value: &'a mut T,
    pub ticks: TicksMut<'a>,
}

pub trait DetectChangesMut: DetectChanges {
    type Inner: ?Sized;
    fn set_changed(&mut self);
    fn set_last_changed(&mut self, last_changed: Tick);
    fn bypass_change_detection(&mut self) -> &mut Self::Inner;
    #[inline]
    fn set_is_neq(&mut self, value: Self::Inner)
    where
        Self::Inner: Sized + PartialEq,
    {
        let old = self.bypass_change_detection();
        if *old != value {
            *old = value;
            self.set_changed();
        }
    }
}

impl<'a> Ticks<'a> {
    #[inline]
    pub unsafe fn from_tick_cells(cells: TickCells<'a>, last_run: Tick, this_run: Tick) -> Self {
        Self {
            added: cells.added.deref(),
            changed: cells.changed.deref(),
            last_run,
            this_run,
        }
    }
}

impl<'a> TicksMut<'a> {
    #[inline]
    pub unsafe fn from_tick_cells(cells: TickCells<'a>, last_run: Tick, this_run: Tick) -> Self {
        Self {
            added: cells.added.deref_mut(),
            changed: cells.changed.deref_mut(),
            last_run,
            this_run,
        }
    }
}

impl<'a> From<TicksMut<'a>> for Ticks<'a> {
    fn from(ticks: TicksMut<'a>) -> Self {
        Ticks {
            added: ticks.added,
            changed: ticks.changed,
            last_run: ticks.last_run,
            this_run: ticks.this_run,
        }
    }
}

impl<'w, T: Resource> Res<'w, T> {
    pub fn clone(this: &Self) -> Self {
        Self {
            value: this.value,
            ticks: this.ticks.clone(),
        }
    }

    pub fn into_inner(self) -> &'w T {
        self.value
    }
}

impl<'w, T: Resource> From<ResMut<'w, T>> for Res<'w, T> {
    fn from(res: ResMut<'w, T>) -> Self {
        Self {
            value: res.value,
            ticks: res.ticks.into(),
        }
    }
}

macro_rules! change_detection_impl {
    ($name:ident<$( $generics:tt),+>,$target:ty,$($traits:ident)?) => {
        impl<$($generics),*:?Sized $(+$traits)?> DetectChanges for $name<$($generics),*>{
            #[inline]
            fn is_added(&self)->bool{
                self.ticks.added.is_newer_than(self.ticks.last_run, self.ticks.this_run)
            }

            #[inline]
            fn is_changed(&self)->bool{
                self.ticks.changed.is_newer_than(self.ticks.last_run,self.ticks.this_run)
            }

            #[inline]
            fn last_changed(&self)->Tick{
                *self.ticks.changed
            }
        }

        impl<$($generics),*: ?Sized $(+$traits)?> Deref for $name<$($generics),*>{
            type Target = $target;

            #[inline]
            fn deref(&self)->&Self::Target{
                self.value
            }
        }

        impl<$($generics),* $(: $traits)?> AsRef<$target> for $name<$($generics),*>{
            #[inline]
            fn as_ref(&self)->&$target{
                self.deref()
            }
        }
    };
}

macro_rules! change_detection_mut_impl {
    ($name:ident < $($generics:tt),+>, $target:ty, $($traits:ident)?) => {
        impl<$($generics),*:?Sized  $(+ $traits)?> DetectChangesMut for $name<$($generics),*>{
            type Inner = $target;

            #[inline]
            fn set_changed(&mut self){
                *self.ticks.changed = self.ticks.this_run
            }

            #[inline]
            fn set_last_changed(&mut self, last_changed: Tick){
                *self.ticks.changed = last_changed
            }

            #[inline]
            fn bypass_change_detection(&mut self)->&mut Self::Inner{
                self.value
            }
        }

        impl<$($generics),* : ?Sized $(+ $traits)?> DerefMut for $name<$($generics),*>{
            #[inline]
            fn deref_mut(&mut self)-> &mut Self::Target{
                self.set_changed();
                self.value
            }
        }

        impl<$($generics),* $(: $traits)?> AsMut<$target> for $name<$($generics),*>{
            #[inline]
            fn as_mut(&mut self)->&mut $target{
                self.deref_mut()
            }
        }
    };
}

macro_rules! impl_method {
    ($name:ident < $( $generics:tt ),+>, $target:ty, $($traits:ident)?) => {
        #[inline]
        pub fn into_inner(mut self)->&'a mut &target{
            self.set_changed()
            self.value
        }

        pub fn reborrow(&mut self) -> Mut<'_,$target>{
            Mut{
                value: self.value,
                ticks: TicksMut{
                    added: self.ticks.added,
                    changed: self.ticks.changed,
                    last_run: self.ticks.last_run,
                    this_run: self.ticks.this_run
                }
            }
        }

        pub fn map_unchanged<U:?Sized>(self, f:impl FnOnce(&mut $target)->&mut U)->Mut<'a,U>{
            Mut{
                value:f(self.value),
                ticks:self.ticks,
            }
        }
    };
}

macro_rules! impl_debug {
    ($name:ident < $( $generics:tt ),+> $($traits:ident)?) => {
        impl<$($generics),* : ?Sized $(+$traits)?> std::fmt::Debug for $name<$($generics),*>
            where T : std::fmt::Debug
        {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result{
                f.debug_tuple(stringify!($name))
                    .field(&self.value)
                    .finish()
            }
        }
    };
}

change_detection_impl!(NonSendMut<'a, T>, T,);
change_detection_mut_impl!(NonSendMut<'a, T>, T,);
