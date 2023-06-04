use std::mem::ManuallyDrop;

use crate::storage::table::Column;

// pub struct ResourceData<const SEND:bool>{
//     column:ManuallyDrop<Column>,
//     type_name:String,
//     id:ArchetypeComponentId,
// }