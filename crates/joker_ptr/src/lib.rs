use std::marker::PhantomData;
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
pub struct MutPtr<'a, A: IsAligned = Aligned>(NonNull<u8>, PhantomData<(&'a mut u8, A)>);

pub struct OwnPtr<'a, A: IsAligned = Aligned>(NonNull<u8>, PhantomData<(&'a mut u8, A)>);

macro_rules! impl_ptr {
    ($ptr:ident) => {
        impl<'a> $ptr<'a, Aligned> {
            pub fn to_unaligned(self) -> $ptr<'a, Unaligned> {
                $ptr(self.0, PhantomData)
            }
        }
    };
}

impl_ptr!(Ptr);
impl_ptr!(MutPtr);
impl_ptr!(OwnPtr);
