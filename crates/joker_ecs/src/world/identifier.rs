#![allow(unused)]

use std::sync::atomic::{AtomicUsize, Ordering};

#[derive(Clone, Copy, PartialEq, Eq, Debug, Hash)]
pub struct WorldId(usize);

static MAX_WORLD_ID: AtomicUsize = AtomicUsize::new(0);

impl WorldId {
    pub fn new() -> Option<Self> {
        MAX_WORLD_ID
            .fetch_update(Ordering::Relaxed, Ordering::Relaxed, |val| {
                val.checked_add(1)
            })
            .map(WorldId)
            .ok()
    }
}

// impl FromWorld for WorldId{
//     #[inline]
//     fn from_world(world:&mut World)->Self{
//         world.id()
//     }
// }