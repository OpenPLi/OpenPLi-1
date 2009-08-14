#ifndef FX_PIG_H
#define FX_PIG_H
extern	void	Fx2SetPig( int x, int y, int width, int height );
extern	void	Fx2ShowPig( int x, int y, int width, int height );
extern	void	Fx2StopPig( void );
extern	void	Fx2PigPause( void );
extern	void	Fx2PigResume( void );

extern	int		fx2_use_pig;			// set before first pig-use !!! def:1

#endif
