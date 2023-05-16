#![allow(unused)]

use joker_ptr::UnsafeCellDeref;
use std::ops::Deref;

use crate::component::{Tick, TickCells};

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

pub struct NonSendMut<'a, T: ?Sized + 'static> {
    pub value: &'a mut T,
    pub ticks: TicksMut<'a>,
}

pub trait DetectChanges {
    fn is_added(&self) -> bool;
    fn is_changed(&self) -> bool;
    fn last_changed(&self) -> Tick;
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

macro_rules! change_detection {
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

change_detection!(NonSendMut<'a, T>, T,);
