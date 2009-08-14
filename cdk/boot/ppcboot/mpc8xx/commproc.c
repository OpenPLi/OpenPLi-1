#include <ppcboot.h>
#include <commproc.h>

void m8xx_cpm_init(uint base, uint size)
{
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    /* Reclaim the DP memory for our use. */
    idata->dp_alloc_base = base;
    idata->dp_alloc_top  = idata->dp_alloc_base + size;
}

/* Allocate some memory from the dual ported ram.  We may want to
 * enforce alignment restrictions, but right now everyone is a good
 * citizen.
 */
uint m8xx_cpm_dpalloc(uint size)
{
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);
    uint	retloc;

    if ((idata->dp_alloc_base + size) >= idata->dp_alloc_top)
	return(CPM_DP_NOSPACE);

    retloc = idata->dp_alloc_base;
    idata->dp_alloc_base += size;

#if 0
    serial_printf("Requested %d bytes\n", size);
    serial_printf("DPRAM BASE=%08x DPRAM TOP=%08x\n",
	idata->dp_alloc_base,
	idata->dp_alloc_top);
    serial_printf("Returned %08x\n", retloc);
#endif

    return(retloc);
}

uint m8xx_cpm_dpbase(void)
{
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);
    uint retloc = idata->dp_alloc_base;

#if 0
    serial_printf("Returned %08x\n", retloc);
#endif
    return  retloc;
}

/* Allocate some memory from the dual ported ram.  We may want to
 * enforce alignment restrictions, but right now everyone is a good
 * citizen.
 */
uint m8xx_cpm_dpalloc_align(uint size, uint align)
{
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);
    uint	retloc, mask = align -1;

    if ((idata->dp_alloc_base + size) >= idata->dp_alloc_top)
	return(CPM_DP_NOSPACE);

    retloc = (idata->dp_alloc_base + mask) & ~mask;
    idata->dp_alloc_base += size;

#if 0
    serial_printf("Requested %d bytes\n", size);
    serial_printf("DPRAM BASE=%08x DPRAM TOP=%08x\n",
	idata->dp_alloc_base,
	idata->dp_alloc_top);
    serial_printf("Returned %08x\n", retloc);
#endif

    return(retloc);
}

uint m8xx_cpm_dpbase_align(uint align)
{
    /* Pointer to initial global data area */
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);
    uint mask = align-1, retloc = (idata->dp_alloc_base + mask) & ~mask;
#if 0
    serial_printf("Returned %08x\n", retloc);
#endif
    return  retloc;
}
