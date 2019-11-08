#![cfg_attr(feature = "mikan-bare", feature(lang_items))]
#![cfg_attr(feature = "mikan-bare", no_std)]
#![no_main]

#[cfg_attr(feature = "mikan-bare", path = "mikan_bare.rs")]
#[cfg_attr(not(feature = "mikan-bare"), path = "mikan_hosted.rs")]
mod mikan_core;

pub static mut O_O: i32 = 42;
pub static Q_Q: &'static str = "=~=";

fn qwq() -> u8 {
    42
}

#[no_mangle]
pub extern fn main() {
    let _a = qwq();
    mikan_core::qwqputs("Hello, world! The answer is\0");
    // Usable in hosted environment
    // println!("The answer is {}", _a);
    loop { }
}
