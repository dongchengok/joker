#![allow(unused)]

use std::{
    alloc::{handle_alloc_error, Layout},
    cell::UnsafeCell,
    num::NonZeroUsize,
    ptr::NonNull,
};

use joker_ptr::{OwningPtr, PtrMut};

mod joker {
    pub mod ptr {
        pub use joker_ptr::*;
    }
    pub mod foundation {
        pub use joker_foundation::*;
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
            unsafe { std::alloc::alloc(new_layout) }
        } else {
            unsafe {
                std::alloc::realloc(
                    self.get_ptr_mut().as_ptr(),
                    array_layout(&self.item_layout, self.capacity)
                        .expect("array layout should be valid"),
                    new_layout.size(),
                )
            }
        };
        self.data = NonNull::new(new_data).unwrap_or_else(|| handle_alloc_error(new_layout));
        self.capacity = new_capacity;
    }

    #[inline]
    pub unsafe fn initialize_unchecked(&mut self, index: usize, value: OwningPtr<'_>) {
        debug_assert!(index < self.len());
        let destination = self.get_unchecked_mut(index);
        std::ptr::copy_nonoverlapping(
            value.as_ptr(),
            destination.as_ptr(),
            self.item_layout.size(),
        );
    }

    #[inline]
    pub unsafe fn replace_unchecked(&mut self, index: usize, value: OwningPtr<'_>) {
        debug_assert!(index < self.len());
        let destination = NonNull::from(self.get_unchecked_mut(index));
        let source = value.as_ptr();
        if let Some(drop) = self.drop {
            let old_len = self.len;
            self.len = 0;
            let old_value = OwningPtr::new(destination);
            let on_unwind = joker::foundation::OnDrop::new(|| drop(value));
            drop(old_value);
            std::mem::forget(on_unwind);
            self.len = old_len;
        }
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
                self.get_unchecked_mut(index).as_ptr(),
                self.get_unchecked_mut(new_len).as_ptr(),
                size,
            );
        }
        self.len = new_len;
        self.get_ptr_mut().byte_add(new_len * size).promote()
    }

    // TODO 这里的拷贝检查没看懂为什么是这样的,而且没有调用析构函数
    #[inline]
    pub unsafe fn swap_remove_unchecked(&mut self, index: usize, ptr: PtrMut<'_>) {
        debug_assert!(index < self.len());
        let last = self.get_unchecked_mut(self.len() - 1).as_ptr();
        let target = self.get_unchecked_mut(index).as_ptr();
        std::ptr::copy_nonoverlapping(target, ptr.as_ptr(), self.item_layout.size());
        std::ptr::copy(last, target, self.item_layout.size());
        self.len -= 1;
    }

    #[inline]
    pub unsafe fn swap_remove_and_drop_unchecked(&mut self, index: usize) {
        debug_assert!(index < self.len());
        let drop = self.drop;
        let value = self.swap_remove_and_forget_unchecked(index);
        if let Some(drop) = drop {
            (drop)(value);
        }
    }

    #[inline]
    pub unsafe fn get_unchecked(&self, index: usize) -> joker::ptr::Ptr<'_> {
        debug_assert!(index < self.len());
        let size = self.item_layout.size();
        // TODO 如果size未对齐的这里就不对了吧
        self.get_ptr().byte_add(size * index)
    }

    #[inline]
    pub unsafe fn get_unchecked_mut(&self, index: usize) -> joker::ptr::PtrMut<'_> {
        debug_assert!(index < self.len());
        let size = self.item_layout.size();
        // TODO 如果size未对齐的这里就不对了吧
        self.get_ptr_mut().byte_add(size * index)
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

impl Drop for BlobVec {
    fn drop(&mut self) {
        self.clear();
        let array_layout =
            array_layout(&self.item_layout, self.capacity).expect("array layout should be valid");
        if array_layout.size() > 0 {
            unsafe {
                std::alloc::dealloc(self.get_ptr_mut().as_ptr(), array_layout);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{array_layout, BlobVec};
    use joker_ptr::OwningPtr;
    use std::{alloc::Layout, cell::RefCell, rc::Rc};

    struct Object {
        id: i32,
    }

    impl Drop for Object {
        fn drop(&mut self) {
            println!("drop:{}", self.id);
        }
    }

    //以类型T的drop函数，清理对象
    unsafe fn drop_ptr<T>(ptr: OwningPtr<'_>) {
        ptr.drop_as::<T>();
    }

    unsafe fn push<T>(blob_vec: &mut BlobVec, value: T) {
        // 把value的数据塞到blob里，value所有权被走s
        OwningPtr::make(value, |ptr| {
            blob_vec.push(ptr);
            //OwningPtr所有权被push拿走
            //let p = ptr.as_mut();
        });
    }

    unsafe fn swap_remove<T>(blob_vec: &mut BlobVec, index: usize) -> T {
        assert!(index < blob_vec.len());
        //这样是不会调用drop函数的
        let value = blob_vec.swap_remove_and_forget_unchecked(index);
        value.read()
    }

    unsafe fn get_mut<T>(blob_vec: &mut BlobVec, index: usize) -> &mut T {
        assert!(index < blob_vec.len());
        blob_vec.get_unchecked_mut(index).deref_mut()
    }

    #[test]
    fn test_resize() {
        let item_layout = Layout::new::<usize>();
        let mut blob_vec = unsafe { BlobVec::new(item_layout, None, 64) };
        unsafe {
            for i in 0..100 {
                push(&mut blob_vec, i as usize);
            }
        }
        assert_eq!(blob_vec.len(), 1_00);
        assert_eq!(blob_vec.capacity(), 1_00);
    }

    #[derive(Debug, Eq, PartialEq, Clone)]
    struct Foo {
        a: u8,
        b: String,
        drop_counter: Rc<RefCell<usize>>,
    }

    impl Drop for Foo {
        fn drop(&mut self) {
            *self.drop_counter.borrow_mut() += 1;
        }
    }

    #[test]
    fn test_blob_vec() {
        let drop_counter = Rc::new(RefCell::new(0));
        {
            let item_layout = Layout::new::<Foo>();
            let drop = drop_ptr::<Foo>;
            let mut blob_vec = unsafe { BlobVec::new(item_layout, Some(drop), 2) };
            unsafe {
                let foo1 = Foo {
                    a: 42,
                    b: "abc".to_string(),
                    drop_counter: drop_counter.clone(),
                };
                push(&mut blob_vec, foo1.clone());
                assert_eq!(blob_vec.len(), 1);
                assert_eq!(get_mut::<Foo>(&mut blob_vec, 0), &foo1);

                let mut foo2 = Foo {
                    a: 7,
                    b: "xyz".to_string(),
                    drop_counter: drop_counter.clone(),
                };

                push(&mut blob_vec, foo2.clone());
                assert_eq!(blob_vec.len(), 2);
                assert_eq!(blob_vec.capacity(), 2);
                assert_eq!(get_mut::<Foo>(&mut blob_vec, 0), &foo1);
                assert_eq!(get_mut::<Foo>(&mut blob_vec, 1), &foo2);

                get_mut::<Foo>(&mut blob_vec, 1).a += 1;
                assert_eq!(get_mut::<Foo>(&mut blob_vec, 1).a, 8);

                let foo3 = Foo {
                    a: 16,
                    b: "123".to_string(),
                    drop_counter: drop_counter.clone(),
                };
                push(&mut blob_vec, foo3.clone());
                assert_eq!(blob_vec.len(), 3);
                assert_eq!(blob_vec.capacity(), 3);

                let last_index = blob_vec.len() - 1;
                let value = swap_remove::<Foo>(&mut blob_vec, last_index);
                assert_eq!(foo3, value);

                assert_eq!(blob_vec.len(), 2);
                assert_eq!(blob_vec.capacity(), 3);

                let value = swap_remove::<Foo>(&mut blob_vec, 0);
                assert_eq!(value, foo1);
                assert_eq!(blob_vec.len(), 1);
                assert_eq!(blob_vec.capacity(), 3);

                foo2.a = 8;
                assert_eq!(get_mut::<Foo>(&mut blob_vec, 0), &foo2);
            }
        }
        assert_eq!(*drop_counter.borrow(), 6);
    }

    #[test]
    fn test_drop_empty_capacity() {
        let item_layout = Layout::new::<Foo>();
        let drop = drop_ptr::<Foo>;
        let _ = unsafe { BlobVec::new(item_layout, Some(drop), 0) };
    }

    #[test]
    fn test_aligned_zst(){
        // #[derive(Component)]
        // #[repr(align(32))]
        // struct Zst;

        // let mut world = World::default();
        // world.spawn(Zst);
        // world.spawn(Zst);
        // world.spawn(Zst);
        // world.spawn_empty();

        // let mut count = 0;

        // let mut q = world.query::<&Zst>();
        // for &Zst in q.iter(&world) {
        //     count += 1;
        // }

        // assert_eq!(count, 3);
    }

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
