#![feature(asm)]


fn main(){
	reboot();
}
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
fn reboot(){
    unsafe {
        asm! ("mov eax, 30"
        :
        :
        : "eax"
        );
        asm! ("mov ebx, [esp + 4]");
        asm! ("int 0x80");
        //asm! ("NOP");
    }
    return;
}