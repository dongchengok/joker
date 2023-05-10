#![allow(unused)]

use std::{
    alloc::{handle_alloc_error, Layout},
    cell::UnsafeCell,
    num::NonZeroUsize,
    ptr::NonNull,
};

use joker_ptr::OwningPtr;

mod joker {
    pub mod ptr {
        pub use joker_ptr::*;
    }
}

fn array_layout(layout: &Layout, n: usize) -> Option<Layout> {
    let (array_layout, offset) = repeat_layout(layout, n)?;
    debug_assert_eq!(layout.size(), offset);
    Some(array_layout)
}

fn repeat_layout(layout: &Layout, n: usize) -> Option<(Layout, usize)> {
    let padded_size = layout.size() + padding_needed_for(layout, layout.align());
    let alloc_size = padded_size.checked_mul(n)?;
    unsafe {
        Some((
            Layout::from_size_align_unchecked(alloc_size, layout.align()),
            padded_size,
        ))
    }
}

/// From <https://doc.rust-lang.org/beta/src/core/alloc/layout.rs.html>
const fn padding_needed_for(layout: &Layout, align: usize) -> usize {
    let len = layout.size();
    let len_rounded_up = len.wrapping_add(align).wrapping_sub(1) & !align.wrapping_sub(1);
    len_rounded_up.wrapping_sub(len)
}

pub struct BlobVec {
    item_layout: Layout,
    capacity: usize,
    len: usize,
    data: NonNull<u8>,
    drop: Option<unsafe fn(joker::ptr::OwningPtr<'_>)>,
}

impl std::fmt::Debug for BlobVec {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("BlobVec")
            .field("item_layout", &self.item_layout)
            .field("capacity", &self.capacity)
            .field("len", &self.len)
            .field("data", &self.data)
            .finish()
    }
}

impl BlobVec {
    pub unsafe fn new(
        item_layout: Layout,
        drop: Option<unsafe fn(joker::ptr::OwningPtr<'_>)>,
        capacity: usize,
    ) -> BlobVec {
        let align = NonZeroUsize::new(item_layout.align()).expect("align must > 0");
        let data = joker::ptr::dangling_with_align(align);
        if item_layout.size() == 0 {
            BlobVec {
                data,
                capacity: usize::MAX,
                len: 0,
                item_layout,
                drop,
            }
        } else {
            let mut blob_vec = BlobVec {
                data,
                capacity: 0,
                len: 0,
                item_layout,
                drop,
            };
            blob_vec.reserve_exact(capacity);
            blob_vec
        }
    }

    #[inline]
    pub fn len(&self) -> usize {
        self.len
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    #[inline]
    pub fn capacity(&self) -> usize {
        self.capacity
    }

    #[inline]
    pub fn layout(&self) -> Layout {
        self.item_layout
    }

    pub fn reserve_exact(&mut self, additional: usize) {
        let available_space = self.capacity - self.len;
        if available_space < additional && self.item_layout.size() > 0 {
            let increment = unsafe { NonZeroUsize::new_unchecked(additional - available_space) };
            unsafe { self.grow_exact(increment) }
        }
    }

    unsafe fn grow_exact(&mut self, increment: NonZeroUsize) {
        debug_assert!(self.item_layout.size() != 0);

        let new_capacity = self.capacity + increment.get();
        let new_layout =
            array_layout(&self.item_layout, new_capacity).expect("array layout should be valid");
        let new_data = if self.capacity == 0 {
            //TODO 为啥要alloc个空的
            unsafe { std::alloc::alloc(new_layout) }
        } else {
            unsafe {
                // std::alloc::realloc(self.get_ptr, layout, new_size)
                unsafe { std::alloc::alloc(new_layout) }
            }
        };
        self.data = NonNull::new(new_data).unwrap_or_else(|| handle_alloc_error(new_layout));
        self.capacity = new_capacity;
    }

    #[inline]
    pub unsafe fn initialize_unchecked(&mut self, index: usize, value: OwningPtr<'_>) {
        // TODO
        debug_assert!(index < self.len());
        // let destination = NonNull::from(self.get_un)
    }

    #[inline]
    pub unsafe fn push(&mut self, value: OwningPtr<'_>) {
        self.reserve_exact(1);
        let index = self.len;
        self.len += 1;
        self.initialize_unchecked(index, value);
    }

    #[inline]
    pub unsafe fn set_len(&mut self, len: usize) {
        debug_assert!(len <= self.capacity());
        self.len = len;
    }

    //这个返回值必须被使用,否则内存就泄露了
    #[inline]
    #[must_use = "The returned pointer should be used to dropped the removed element"]
    pub unsafe fn swap_remove_and_forget_unchecked(&mut self, index: usize) -> OwningPtr<'_> {
        debug_assert!(index < self.len());
        let new_len = self.len - 1;
        let size = self.item_layout.size();
        if index != new_len {
            std::ptr::swap_nonoverlapping::<u8>(
                self.get_unchecked_mut(index).as_ptr,
                self.get_unchecked_mut(new_len).as_ptr(),
                size,
            );
        }
        self.len = new_len;
        self.get_ptr_mut().byte_add(new_len * size).promote()
    }

    #[inline]
    pub fn get_ptr(&self) -> joker::ptr::Ptr<'_> {
        unsafe { joker::ptr::Ptr::new(self.data) }
    }

    #[inline]
    pub fn get_ptr_mut(&self) -> joker::ptr::PtrMut<'_> {
        unsafe { joker::ptr::PtrMut::new(self.data) }
    }

    #[inline]
    pub unsafe fn get_slice<T>(&self) -> &[UnsafeCell<T>] {
        std::slice::from_raw_parts(self.data.as_ptr() as *const UnsafeCell<T>, self.len)
    }

    #[inline]
    pub fn clear(&mut self) {
        let len = self.len;
        self.len = 0;
        if let Some(drop) = self.drop {
            let size = self.item_layout.size();
            for i in 0..len {
                let item = unsafe { self.get_ptr_mut().byte_add(i * size).promote() };
                unsafe { drop(item) };
            }
        }
    }
}

impl Drop for BlobVec{
    fn drop(&mut self) {
        self.clear();
        let array_layout = array_layout(&self.item_layout, self.capacity).expect("array layout should be valid");
        if array_layout.size()>0{
            unsafe{
                std::alloc::dealloc(self.get_ptr_mut().as_ptr(), array_layout);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::array_layout;
    use std::alloc::Layout;

    #[test]
    fn test_wrapping() {
        assert_eq!(99usize, 100usize.wrapping_add(usize::MAX));
    }

    #[test]
    fn test_array_layout() {
        let layout = Layout::from_size_align(8, 8).unwrap();
        let new_layout = array_layout(&layout, 200).unwrap();
        assert_eq!(new_layout.size(), 200 * 8);
        assert_eq!(new_layout.align(), 8);
    }
}
