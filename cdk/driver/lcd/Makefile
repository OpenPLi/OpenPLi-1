DRIVER_TOPDIR = ..

include $(DRIVER_TOPDIR)/Config.make

obj-m		:= lcd.o

LCD_OBJS	:= lcd-console.o lcd-font.o lcd-ks0713.o

lcd.o: $(LCD_OBJS)
	$(LD) -r -o $@ $(LCD_OBJS)

include $(DRIVER_TOPDIR)/Rules.make
