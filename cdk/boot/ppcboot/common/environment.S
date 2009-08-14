#include <config.h>

#ifdef	CFG_FLASH_ENV_ADDR
	.set	 env_not_stored_with_text, 1
#else

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#if defined(CONFIG_FADS) || defined(CONFIG_HYMOD)
	.section	".ppcenv"
#else
	.text
#endif
	.globl	environment
environment:
#ifdef	CONFIG_BOOTARGS
	.ascii	"bootargs="
	.ascii		CONFIG_BOOTARGS
	.ascii		"\0"
#endif
#ifdef	CONFIG_BOOTCOMMAND
	.ascii	"bootcmd="
	.ascii		CONFIG_BOOTCOMMAND
	.ascii		"\0"
#endif
#if (CONFIG_BOOTDELAY >= 0)
	.ascii	"bootdelay="
	.ascii		MK_STR(CONFIG_BOOTDELAY)
	.ascii		"\0"
#endif
#if (CONFIG_BAUDRATE >= 0)
	.ascii	"baudrate="
	.ascii		MK_STR(CONFIG_BAUDRATE)
	.ascii		"\0"
#endif
	.ascii	"loads_echo="
	.ascii		MK_STR(CONFIG_LOADS_ECHO)
	.ascii		"\0"
#ifdef	CONFIG_ETHADDR
	.ascii	"ethaddr="
	.ascii		MK_STR(CONFIG_ETHADDR)
	.ascii		"\0"
#endif
#ifdef	CONFIG_IPADDR
	.ascii	"ipaddr="
	.ascii		MK_STR(CONFIG_IPADDR)
	.ascii		"\0"
#endif
#ifdef	CONFIG_SERVERIP
	.ascii	"serverip="
	.ascii		MK_STR(CONFIG_SERVERIP)
	.ascii		"\0"
#endif


	/* terminate list of environment strings */
	.ascii	"\0"
	. = environment + CFG_FLASH_ENV_SIZE
.L_end:
#if defined(CONFIG_FADS)
	.text
#endif
	.globl  env_size
env_size:
	.long	.L_end - environment
	.globl	env_offset
	.set	env_offset, CFG_FLASH_ENV_OFFSET

#endif	/* CFG_FLASH_ENV_ADDR */
