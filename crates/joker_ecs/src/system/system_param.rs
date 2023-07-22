#![allow(unused)]

use std::ops::Deref;

use crate::world::FromWorld;

pub trait Resource: Send + Sync + 'static {}

// pub unsafe trait SystemParam : Sized{
//     type State: Send + Sync + 'static;
//     type Item<'world,'state>:SystemParam<State=Self::State>;
//     fn init_state(world:&mut World, system_meta:&mut SystemMeta)->Self::State;
// }

// pub unsafe trait ReadOnlySystemParam : SystemParam{}

pub struct Local<'s, T:FromWorld + Send + 'static>(pub(crate)&'s mut T);

impl<'s, T:FromWorld + Send + Sync + 'static> Deref for Local<'s,T>{
    type Target = T;

    #[inline]
    fn deref(&self) -> &Self::Target {
        self.0
    }
}