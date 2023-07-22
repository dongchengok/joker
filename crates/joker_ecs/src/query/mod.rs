mod access;

pub(crate) trait DebugCheckedUnwrap {
    type Item;
    unsafe fn debug_checked_unwrap(self) -> Self::Item;
}

#[cfg(debug_assertions)]
impl<T> DebugCheckedUnwrap for Option<T> {
    type Item = T;

    #[inline(always)]
    #[track_caller]
    unsafe fn debug_checked_unwrap(self) -> Self::Item {
        if let Some(inner) = self {
            inner
        } else {
            unreachable!()
        }
    }
}

#[cfg(not(debug_assertions))]
impl<T> DebugCheckedUnwrap for Option<T>{
    type Item = T;

    #[inline(always)]
    unsafe fn debug_checked_unwrap(self)->Self::Item{
        if let Some(inner) = self{
            inner
        }else{
            std::hint::unreachable_unchecked()
        }
    }
}