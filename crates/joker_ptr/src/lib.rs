use core::fmt::{self, Formatter, Pointer};
use std::cell::UnsafeCell;
use std::marker::PhantomData;
use std::mem::ManuallyDrop;
use std::num::NonZeroUsize;
use std::ptr::NonNull;

#[derive(Clone, Copy)]
pub struct Aligned;

#[derive(Clone, Copy)]
pub struct Unaligned;

/// 因为不是pub的，所以外部无法继续继承aligned了
mod sealed {
    use super::{Aligned, Unaligned};

    pub trait Sealed {}
    impl Sealed for Aligned {}
    impl Sealed for Unaligned {}
}

pub trait IsAligned: sealed::Sealed {}
impl IsAligned for Aligned {}
impl IsAligned for Unaligned {}

/// 不允许修改的指针，所以直接使用拷贝语义就行了
#[derive(Clone, Copy)]
pub struct Ptr<'a, A: IsAligned = Aligned>(NonNull<u8>, PhantomData<(&'a u8, A)>);

/// 允许修改内容的指针，所以不能使用拷贝的语义，需要走借用的机制
pub struct PtrMut<'a, A: IsAligned = Aligned>(NonNull<u8>, PhantomData<(&'a mut u8, A)>);

pub struct OwningPtr<'a, A: IsAligned = Aligned>(NonNull<u8>, PhantomData<(&'a mut u8, A)>);

macro_rules! impl_ptr {
    ($ptr:ident) => {
        impl<'a> $ptr<'a, Aligned> {
            pub fn to_unaligned(self) -> $ptr<'a, Unaligned> {
                $ptr(self.0, PhantomData)
            }
        }

        impl<'a, A: IsAligned> From<$ptr<'a, A>> for NonNull<u8> {
            fn from(ptr: $ptr<'a, A>) -> Self {
                ptr.0
            }
        }

        impl<A: IsAligned> $ptr<'_, A> {
            #[inline]
            pub unsafe fn byte_offset(self, count: isize) -> Self {
                Self(
                    NonNull::new_unchecked(self.as_ptr().offset(count)),
                    PhantomData,
                )
            }

            #[inline]
            pub unsafe fn byte_add(self, count: usize) -> Self {
                Self(
                    NonNull::new_unchecked(self.as_ptr().add(count)),
                    PhantomData,
                )
            }
        }

        impl<A: IsAligned> Pointer for $ptr<'_, A> {
            #[inline]
            fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
                Pointer::fmt(&self.0, f)
            }
        }
    };
}

impl_ptr!(Ptr);
impl_ptr!(PtrMut);
impl_ptr!(OwningPtr);

impl<'a, A: IsAligned> Ptr<'a, A> {
    #[inline]
    pub unsafe fn new(inner: NonNull<u8>) -> Self {
        Self(inner, PhantomData)
    }

    /// 这个转化是安全的，因为多次转化会夺取所有权
    #[inline]
    pub unsafe fn assert_unique(self) -> PtrMut<'a, A> {
        PtrMut(self.0, PhantomData)
    }

    #[inline]
    pub unsafe fn deref<T>(self) -> &'a T {
        &*self.as_ptr().cast::<T>().debug_ensure_aligned()
    }

    #[inline]
    #[allow(clippy::wrong_self_convention)]
    pub fn as_ptr(self) -> *mut u8 {
        self.0.as_ptr()
    }
}

impl<'a, T> From<&'a T> for Ptr<'a> {
    #[inline]
    fn from(val: &'a T) -> Self {
        unsafe { Self::new(NonNull::from(val).cast()) }
    }
}

impl<'a, A: IsAligned> PtrMut<'a, A> {
    #[inline]
    pub unsafe fn new(inner: NonNull<u8>) -> Self {
        Self(inner, PhantomData)
    }

    #[inline]
    pub unsafe fn promote(self) -> OwningPtr<'a, A> {
        OwningPtr(self.0, PhantomData)
    }

    #[inline]
    pub unsafe fn deref_mut<T>(self) -> &'a mut T {
        &mut *self.as_ptr().cast::<T>().debug_ensure_aligned()
    }

    //这块的生命周期没看懂，需要测试一下，as_ptr之后看起来指针就已经非法了
    #[inline]
    #[allow(clippy::wrong_self_convention)]
    pub fn as_ptr(&self) -> *mut u8 {
        self.0.as_ptr()
    }

    //大概明白了，重点是NonNull有Copy的trait，所以复制之后依然合法
    #[inline]
    pub fn reborrow(&mut self) -> PtrMut<'_, A> {
        unsafe { PtrMut::new(self.0) }
    }

    #[inline]
    pub fn as_ref(&self) -> Ptr<'_, A> {
        unsafe { Ptr::new(self.0) }
    }
}

impl<'a, T> From<&'a mut T> for PtrMut<'a> {
    #[inline]
    fn from(val: &'a mut T) -> Self {
        unsafe { Self::new(NonNull::from(val).cast()) }
    }
}

//TODO 这块还没看懂
impl<'a> OwningPtr<'a> {
    #[inline]
    pub fn make<T, F: FnOnce(OwningPtr<'_>) -> R, R>(val: T, f: F) -> R {
        let mut temp = ManuallyDrop::new(val);
        f(unsafe { PtrMut::from(&mut *temp).promote() })
    }
}

impl<'a, A: IsAligned> OwningPtr<'a, A> {
    #[inline]
    pub unsafe fn new(inner: NonNull<u8>) -> Self {
        Self(inner, PhantomData)
    }

    #[inline]
    pub unsafe fn read<T>(self) -> T {
        self.as_ptr().cast::<T>().debug_ensure_aligned().read()
    }

    #[inline]
    pub unsafe fn drop_as<T>(self) {
        self.as_ptr()
            .cast::<T>()
            .debug_ensure_aligned()
            .drop_in_place();
    }

    #[inline]
    #[allow(clippy::wrong_self_convention)]
    pub fn as_ptr(&self) -> *mut u8 {
        self.0.as_ptr()
    }

    #[inline]
    pub fn as_ref(&self) -> Ptr<'_, A> {
        unsafe { Ptr::new(self.0) }
    }

    pub fn as_mut(&mut self) -> PtrMut<'_, A> {
        unsafe { PtrMut::new(self.0) }
    }
}

impl<'a> OwningPtr<'a, Unaligned> {
    //我自己加了个inline
    #[inline]
    pub unsafe fn read_unaligned<T>(self) -> T {
        self.as_ptr().cast::<T>().read_unaligned()
    }
}

pub struct ThinSlicePtr<'a, T> {
    ptr: NonNull<T>,
    #[cfg(debug_assertions)]
    len: usize,
    _marker: PhantomData<&'a [T]>,
}

impl<'a, T> ThinSlicePtr<'a, T> {
    #[inline]
    pub unsafe fn get(self, index: usize) -> &'a T {
        #[cfg(debug_assertions)]
        debug_assert!(index < self.len);
        &*self.ptr.as_ptr().add(index)
    }
}

impl<'a, T> Clone for ThinSlicePtr<'a, T> {
    fn clone(&self) -> Self {
        //因为有copy，所以这个才合法
        *self
    }
}

impl<'a, T> Copy for ThinSlicePtr<'a, T> {}

impl<'a, T> From<&'a [T]> for ThinSlicePtr<'a, T> {
    #[inline]
    fn from(slice: &'a [T]) -> Self {
        let ptr = slice.as_ptr() as *mut T;
        Self {
            ptr: unsafe { NonNull::new_unchecked(ptr.debug_ensure_aligned()) },
            #[cfg(debug_assertions)]
            len: slice.len(),
            _marker: PhantomData,
        }
    }
}

pub fn dangling_with_align(align: NonZeroUsize) -> NonNull<u8> {
    #[cfg(debug_assertions)]
    debug_assert!(align.is_power_of_two(), "Alignment must be power of two.");
    unsafe { NonNull::new_unchecked(align.get() as *mut u8) }
}

mod private {
    use core::cell::UnsafeCell;

    pub trait SealedUnsafeCell {}
    //这几行代码没看懂还
    impl<'a, T> SealedUnsafeCell for &'a UnsafeCell<T> {}
}

//这个肯定是不可继承的
pub trait UnsafeCellDeref<'a, T>: private::SealedUnsafeCell {
    unsafe fn deref_mut(self) -> &'a mut T;
    unsafe fn deref(self) -> &'a T;
    unsafe fn read(self) -> T
    where
        T: Copy;
}

//UnsafeCell看起来没有copy，所以自己可能会被消耗掉，不允许用了
impl<'a, T> UnsafeCellDeref<'a,T> for &'a UnsafeCell<T>{
    unsafe fn deref_mut(self) -> &'a mut T {
        &mut *self.get()
    }

    unsafe fn deref(self) -> &'a T {
        &*self.get()
    }

    unsafe fn read(self) -> T
    where
        T: Copy {
        self.get().read()
    }
}

trait DebugEnsureAligned {
    fn debug_ensure_aligned(self) -> Self;
}

//debug_assertions 是断言宏的脚本，debug下生效 https://github.com/rust-lang/miri
//miri 是执行测试的一个环境 https://github.com/rust-lang/miri
#[cfg(all(debug_assertions, not(miri)))]
impl<T: Sized> DebugEnsureAligned for *mut T {
    #[track_caller]
    fn debug_ensure_aligned(self) -> Self {
        let align = core::mem::align_of::<T>();
        //这里的self应该是调用了一个deref?
        assert!(
            self as usize & (align - 1) == 0,
            "pointer is not aligned. Address {:p} does not have alignment {} for type {}",
            self,
            align,
            core::any::type_name::<T>(),
        );
        self
    }
}

#[cfg(any(not(debug_assertions), miri))]
impl<T: Sized> DebugEnsureAligned for *mut T {
    #[inline(always)]
    fn debug_ensure_aligned(self) -> Self {
        self
    }
}
