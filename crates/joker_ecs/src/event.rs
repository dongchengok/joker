#![allow(unused)]

use std::{
    fmt,
    hash::{Hash, Hasher},
    marker::PhantomData,
};

pub trait Event: Send + Sync + 'static {}
impl<T> Event for T where T: Send + Sync + 'static {}

pub struct EventId<E: Event> {
    pub id: usize,
    _marker: PhantomData<E>,
}

impl<E: Event> Copy for EventId<E> {}
impl<E: Event> Clone for EventId<E> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<E: Event> fmt::Debug for EventId<E> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "event<{}>#{}",
            std::any::type_name::<E>().split("::").last().unwrap(),
            self.id
        )
    }
}

impl<E: Event> fmt::Display for EventId<E> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> fmt::Result {
        <Self as fmt::Debug>::fmt(self, f)
    }
}

impl<E: Event> PartialEq for EventId<E> {
    fn eq(&self, other: &Self) -> bool {
        self.id == other.id
    }
}

impl<E: Event> Eq for EventId<E> {}

impl<E: Event> PartialOrd for EventId<E> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl<E: Event> Ord for EventId<E> {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.id.cmp(&other.id)
    }
}

impl<E: Event> Hash for EventId<E> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        Hash::hash(&self.id, state);
    }
}

#[derive(Debug)]
struct EventInstance<E: Event> {
    pub event_id: EventId<E>,
    pub event: E,
}

// #[derive(Debug,Resource)]
// pub struct Events<E:Event>
// {

// }