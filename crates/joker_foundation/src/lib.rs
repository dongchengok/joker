fn main() {
    println!("Hello, world!");
}

pub struct Entity {
    generation: u32,
    index: u32,
}

pub enum AllocWithoutReplacement {
    Exist
}
