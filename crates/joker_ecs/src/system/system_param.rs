#![allow(unused)]

pub trait Resource: Send + Sync + 'static {}

// pub unsafe trait SystemParam : Sized{
//     type State: Send + Sync + 'static;
//     type Item<'world,'state>:SystemParam<State=Self::State>;
//     fn init_state(world:&mut World, system_meta:&mut SystemMeta)->Self::State;
// }

// pub unsafe trait ReadOnlySystemParam : SystemParam{}