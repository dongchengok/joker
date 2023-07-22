use std::any::TypeId;

pub mod archetype;
pub mod storage;
pub mod entity;
pub mod component;
pub mod world;
pub mod system;
pub mod bundle;
pub mod event;
pub mod change_detection;
pub mod removal_detection;
pub mod query;

pub use joker_foundation::all_tuples;

type TypeIdMap<V> = rustc_hash::FxHashMap<TypeId, V>;