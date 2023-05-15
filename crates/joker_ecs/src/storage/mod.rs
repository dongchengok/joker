#![allow(unused)]

pub mod blob_vec;
pub mod sparse_set;
pub mod table;

pub use sparse_set::*;

#[derive(Default)]
pub struct Storages{
    pub sparse_sets:SparseSets,
    // pub tales:Tables,
    // pub resources:Resources<true>,
    // pub non_send_rsources:Resources<false>,
}