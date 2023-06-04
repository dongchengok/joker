#![allow(unused)]

use std::mem::ManuallyDrop;
use hashbrown::hash_map::DefaultHashBuilder;

pub type HashMap<K, V> = hashbrown::HashMap<K, V, DefaultHashBuilder>;
pub type HashSet<T> = hashbrown::HashSet<T, DefaultHashBuilder>;

pub type Entry<'a, K, V> = hashbrown::hash_map::Entry<'a, K, V, DefaultHashBuilder>;

pub struct OnDrop<F: FnOnce()> {
    callback: ManuallyDrop<F>,
}

impl<F: FnOnce()> OnDrop<F> {
    pub fn new(callback: F) -> Self {
        Self {
            callback: ManuallyDrop::new(callback),
        }
    }
}

impl<F: FnOnce()> Drop for OnDrop<F> {
    fn drop(&mut self) {
        let callback = unsafe { ManuallyDrop::take(&mut self.callback) };
        callback();
    }
}

#[cfg(test)]
mod tests {
    use std::mem::ManuallyDrop;

    use crate::HashSet;

    #[derive(Debug)]
    struct Object {
        id: i32,
    }

    impl Drop for Object {
        fn drop(&mut self) {
            println!("drop:{}", self.id);
        }
    }

    #[test]
    fn test_drop_into() {
        // let a = HashSet::new::<i32>();
        let b:hashbrown::HashSet<i32> = hashbrown::HashSet::new();
        // let a = joker_foundation::HashSet::new::<i32>();

        let obj = Object { id: 1i32 };
        //转移所有权
        let obj_drop = ManuallyDrop::new(obj);
        //obj就不能访问了
        // println!("{:?}",&obj);
        //再把obj拿回来
        let obj_into = ManuallyDrop::into_inner(obj_drop);
        //obj_drop就没有所有权了
        //let id = obj_drop.id;
        //obj_into会自动调用析构
    }

    #[test]
    fn test_drop_take() {
        let obj = Object { id: 1i32 };
        //转移所有权
        let mut obj_drop = ManuallyDrop::new(obj);
        //obj就不能访问了
        // println!("{:?}",&obj);
        //再把obj拿回来,obj_drop还是合法的
        let obj_into = unsafe { ManuallyDrop::take(&mut obj_drop) };
        //没有拿走所有权
        let id = obj_drop.id;
        //obj_into会自动调用析构
    }
}
