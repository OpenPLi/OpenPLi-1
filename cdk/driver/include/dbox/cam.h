#ifdef __KERNEL__
extern int cam_reset(void);
extern int cam_write_message( char * buf, size_t count );
extern int cam_read_message( char * buf, size_t count );
#endif
