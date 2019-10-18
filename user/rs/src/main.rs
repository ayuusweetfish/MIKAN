#![feature(lang_items)]
#![no_main]
#![no_std]

use core::panic::PanicInfo;

pub static mut O_O: i32 = 42;
pub static Q_Q: &'static str = "=~=";

fn qwq() -> u8 {
    42
}

#[no_mangle]
pub extern fn main() {
    let _a = qwq();
    loop { }
}

#[lang = "eh_personality"]
extern fn eh_personality() { }

#[panic_handler]
extern fn panic_handler(_info: &PanicInfo) -> ! { loop { } }
