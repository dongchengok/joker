#![allow(unused)]

use std::borrow::Cow;

#[derive(Clone)]
pub struct SystemMeta {
    pub(crate) name: Cow<'static, str>,
    // pub(crate) component_access_set: Filtered
}
