QEMU = qemu-system-i386
os.img:	
	@cd app; make
	@cd autoqed; make
	@touch dummy.img
	@python3 utils/gen_file.py dummy.img kernel/include/common/file_info.h app/app.elf autoqed/autoqed.elf
	@cd bootloader; make
	@cd kernel; make
	@rm -rf dummy.img
	@cat bootloader/bootloader.bin kernel/kMain.elf > os.img
	@python3 utils/gen_file.py os.img kernel/include/common/file_info.h app/app.elf autoqed/autoqed.elf

play:clean os.img
	$(QEMU) -serial stdio os.img -no-reboot

debug: os.img
	$(QEMU) -serial stdio -s -S os.img

gdb:
	gdb -n -x ./.gdbconf/.gdbinit

view: clean os.img
	$(QEMU) --nographic os.img

clean:
	@cd bootloader; make clean
	@cd kernel; make clean
	@cd app; make clean
	@cd autoqed; make clean
	rm -f os.img
