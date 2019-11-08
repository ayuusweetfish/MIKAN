extern crate libc;

pub fn qwqputs(_s: &str) {
    unsafe {
        libc::puts(_s.as_ptr() as *const _);
    }
}
