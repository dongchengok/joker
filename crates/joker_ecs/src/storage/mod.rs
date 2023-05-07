use std::{alloc::Layout, ptr::NonNull};

mod blob_vec;
mod sparse_set;
mod table;

pub struct BlobVec {
    item_layout: Layout,
    capacity: usize,
    len: usize,
    data: NonNull<u8>,
    //drop:Option<unsafe fn(OwingPtr<'_>)>,
}
