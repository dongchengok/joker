#![allow(unused)]

use crate::{entity::Entity, component::Component};

#[derive(Debug, Clone)]
pub struct RemovedComponentEntity(Entity);

impl From<RemovedComponentEntity> for Entity {
    fn from(value: RemovedComponentEntity) -> Self {
        value.0
    }
}

// pub struct RemovedComponentReader<T:Component>{
//     reader:ManualEventReader<
// }