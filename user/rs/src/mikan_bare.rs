use core::panic::PanicInfo;

#[lang = "eh_personality"]
extern fn eh_personality() { }

#[panic_handler]
extern fn panic_handler(_info: &PanicInfo) -> ! { loop { } }

pub fn qwqputs(_s: &str) {
}
