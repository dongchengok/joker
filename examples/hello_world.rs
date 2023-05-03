use joker_rhi::Device;

fn main(){
    let inst = joker::rhi::initialize();
    let device = inst.create_device();
}