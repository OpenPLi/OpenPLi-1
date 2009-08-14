#include <ppcboot.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)

#include <kgdb.h>
#include <asm/signal.h>
#include <asm/processor.h>

#define PC_REGNUM 64
#define SP_REGNUM 1

void breakinst(void);

int
kgdb_setjmp(long *buf)
{
	asm ("mflr 0; stw 0,0(%0);"
	     "stw 1,4(%0); stw 2,8(%0);"
	     "mfcr 0; stw 0,12(%0);"
	     "stmw 13,16(%0)"
	     : : "r" (buf));
	/* XXX should save fp regs as well */
	return 0;
}

void
kgdb_longjmp(long *buf, int val)
{
	if (val == 0)
		val = 1;
	asm ("lmw 13,16(%0);"
	     "lwz 0,12(%0); mtcrf 0x38,0;"
	     "lwz 0,0(%0); lwz 1,4(%0); lwz 2,8(%0);"
	     "mtlr 0; mr 3,%1"
	     : : "r" (buf), "r" (val));
}

static inline unsigned long
get_msr(void)
{
	unsigned long msr;
	asm volatile("mfmsr %0" : "=r" (msr):);
	return msr;
}

static inline void
set_msr(unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}

/* Convert the SPARC hardware trap type code to a unix signal number. */
/*
 * This table contains the mapping between PowerPC hardware trap types, and
 * signals, which are primarily what GDB understands.
 */
static struct hard_trap_info
{
	unsigned int tt;		/* Trap type code for powerpc */
	unsigned char signo;		/* Signal that we map this trap into */
} hard_trap_info[] = {
	{ 0x200, SIGSEGV },			/* machine check */
	{ 0x300, SIGSEGV },			/* address error (store) */
	{ 0x400, SIGBUS },			/* instruction bus error */
	{ 0x500, SIGINT },			/* interrupt */
	{ 0x600, SIGBUS },			/* alingment */
	{ 0x700, SIGTRAP },			/* breakpoint trap */
	{ 0x800, SIGFPE },			/* fpu unavail */
	{ 0x900, SIGALRM },			/* decrementer */
	{ 0xa00, SIGILL },			/* reserved */
	{ 0xb00, SIGILL },			/* reserved */
	{ 0xc00, SIGCHLD },			/* syscall */
	{ 0xd00, SIGTRAP },			/* single-step/watch */
	{ 0xe00, SIGFPE },			/* fp assist */
	{ 0, 0}				/* Must be last */
};

static int
computeSignal(unsigned int tt)
{
	struct hard_trap_info *ht;

	for (ht = hard_trap_info; ht->tt && ht->signo; ht++)
		if (ht->tt == tt)
			return ht->signo;

	return SIGHUP;         /* default for things we don't know about */
}

void
kgdb_enter(struct pt_regs *regs, kgdb_data *kdp)
{
	unsigned long msr;

	kdp->private[0] = msr = get_msr();
	set_msr(msr & ~MSR_EE);	/* disable interrupts */

	if (regs->nip == (unsigned long)breakinst) {
		/* Skip over breakpoint trap insn */
		regs->nip += 4;
	}

	/* reply to host that an exception has occurred */
	kdp->sigval = computeSignal(regs->trap);

	kdp->nregs = 2;

	kdp->regs[0].num = PC_REGNUM;
	kdp->regs[0].val = regs->nip;

	kdp->regs[1].num = SP_REGNUM;
	kdp->regs[1].val = regs->gpr[SP_REGNUM];
}

void
kgdb_exit(struct pt_regs *regs, kgdb_data *kdp)
{
	unsigned long msr = kdp->private[0];

	if (kdp->extype & KGDBEXIT_WITHADDR)
		regs->nip = kdp->exaddr;

	switch (kdp->extype & KGDBEXIT_TYPEMASK) {

	case KGDBEXIT_KILL:
	case KGDBEXIT_CONTINUE:
		set_msr(msr);
		break;

	case KGDBEXIT_SINGLE:
		regs->msr |= MSR_SE;
#if 0
		set_msr(msr | MSR_SE);
#endif
		break;
	}
}

int
kgdb_trap(struct pt_regs *regs)
{
	return (regs->trap);
}

/* return the value of the CPU registers.
 * some of them are non-PowerPC names :(
 * they are stored in gdb like:
 * struct {
 *     u32 gpr[32];
 *     f64 fpr[32];
 *     u32 pc, ps, cnd, lr; (ps=msr)
 *     u32 cnt, xer, mq;
 * }
 */

#define SPACE_REQUIRED	((32*4)+(32*8)+(6*4))

int
kgdb_getregs(struct pt_regs *regs, char *buf, int max)
{
	int i;
	unsigned long *ptr = (unsigned long *)buf;

	if (max < SPACE_REQUIRED)
		kgdb_error(KGDBERR_NOSPACE);

	/* General Purpose Regs */
	for (i = 0; i < 32; i++)
		*ptr++ = regs->gpr[i];

	/* Floating Point Regs - FIXME */
	/*ptr = mem2hex((char *)??, ptr, 32 * 8);*/
	for (i = 0; i < 32; i++) {
		*ptr++ = 0;
		*ptr++ = 0;
	}

	/* pc, msr, cr, lr, ctr, xer, (mq is unused) */
	*ptr++ = regs->nip;
	*ptr++ = regs->msr;
	*ptr++ = regs->ccr;
	*ptr++ = regs->link;
	*ptr++ = regs->ctr;
	*ptr++ = regs->xer;

	return (SPACE_REQUIRED);
}

/* set the value of the CPU registers */

void
kgdb_putregs(struct pt_regs *regs, char *buf, int length)
{
	int i;
	unsigned long *ptr = (unsigned long *)buf;

	if (length < SPACE_REQUIRED)
		kgdb_error(KGDBERR_NOSPACE);

	/*
	 * If the stack pointer has moved, you should pray.
	 * (cause only god can help you).
	 */

	/* General Purpose Regs */
	for (i = 0; i < 32; i++)
		regs->gpr[i] = *ptr++;

	/* Floating Point Regs - FIXME?? */
	/*ptr = hex2mem(ptr, ??, 32 * 8);*/
	ptr += 32*2;

	/* pc, msr, cr, lr, ctr, xer, (mq is unused) */
	regs->nip = *ptr++;
	regs->msr = *ptr++;
	regs->ccr = *ptr++;
	regs->link = *ptr++;
	regs->ctr = *ptr++;
	regs->xer = *ptr++;
}

/* This function will generate a breakpoint exception.  It is used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

void
kgdb_breakpoint(void)
{
	asm("	.globl breakinst
	     breakinst: .long 0x7d821008
            ");
}

#endif /* CFG_CMD_KGDB */
