#ifndef MAZE_H
#define MAZE_H

extern	void	DrawBoard( void );
extern	void	BoardInitialize( void );
extern	void	DrawGameOver( void );
extern	void	MoveSide( void );
extern	int		NextItem( void );
extern	int		PutPuzIntoBoard( void );
extern	int		FallDown( void );
extern	void	RemoveCompl( void );

#endif
