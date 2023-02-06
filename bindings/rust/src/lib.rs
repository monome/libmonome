#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

//! # libmonome bindings for Rust
//! ### Example:
//! ```
//! // Find the device path of your monome device, e.g. on linux it may be:
//! let path = "/dev/ttyUSB0";
//! let path = std::ffi::CString::new(path).unwrap();
//!
//! // Open the device:
//! let mut monome = unsafe { monome_sys::monome_open(path.as_ptr()) };
//! if !monome.is_null() {
//!   // Switch on an LED:
//!   unsafe { monome_sys::monome_led_level_set(monome, 0, 0, 15) };
//! }
//! ```

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
