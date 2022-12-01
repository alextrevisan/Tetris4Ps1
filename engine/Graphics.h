#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <psxgte.h>
#include <psxgpu.h>

class Graphics
{
private:

    u_long    *_orderingTable[2];
    u_char    *_primitives[2];
    u_char    *_nextPrimitive;

    int _orderingTableCount;
    int _doubleBufferIndex;
    
    DISPENV    _displayEnvironment[2];
    DRAWENV _drawEnvironment[2];
    
public:

    Graphics( int orderingTableLength = 8, int primitiveLength = 8192 )
    {
        ResetGraph( 0 );
        _orderingTable[0] = new u_long[orderingTableLength];
        _orderingTable[1] = new u_long[orderingTableLength];
        
        _doubleBufferIndex = 0;
        _orderingTableCount = orderingTableLength;
        ClearOTagR( _orderingTable[0], _orderingTableCount );
        ClearOTagR( _orderingTable[1], _orderingTableCount );
        
        _primitives[0] = new u_char[primitiveLength];
        _primitives[1] = new u_char[primitiveLength];
        
        _nextPrimitive = _primitives[0];
        
        printf( "Graphics::Graphics: Buffers allocated.\n" );
        
    }
    
    virtual ~Graphics()
    {
        delete[] _orderingTable[0];
        delete[] _orderingTable[1];
        
        delete[] _primitives[0];
        delete[] _primitives[1];
        
        printf( "Graphics::Graphics: Buffers freed.\n" );
    }
    
    void SetRes( int w, int h )
    {
        SetDefDispEnv( &_displayEnvironment[0], 0, h, w, h );
        SetDefDispEnv( &_displayEnvironment[1], 0, 0, w, h );
        
        _displayEnvironment[0].isinter = 1;
        
        _displayEnvironment[1].isinter = 1;

        SetDefDrawEnv( &_drawEnvironment[0], 0, 0, w, h );
        SetDefDrawEnv( &_drawEnvironment[1], 0, h, w, h );
        
        setRGB0( &_drawEnvironment[0], 63, 0, 127 );
        _drawEnvironment[0].isbg = 1;
        _drawEnvironment[0].dtd = 1;
        setRGB0( &_drawEnvironment[1], 63, 0, 127 );
        _drawEnvironment[1].isbg = 1;
        _drawEnvironment[1].dtd = 1;
        
        
        PutDispEnv( &_displayEnvironment[0] );
        PutDrawEnv( &_drawEnvironment[0] );

        SetDispMask(1);
    }
    
    void IncPri( int bytes )
    {
        _nextPrimitive += bytes;
    }

    template<typename T>
    void IncPri()
    {
        _nextPrimitive += sizeof(T);
    }
    
    void SetPri( u_char *ptr )
    {
        _nextPrimitive = ptr;
    }
    
    u_char *GetNextPri( void )
    {
        return( _nextPrimitive );
    }

    template<typename T>
    T *GetNextPri( )
    {
        return (T*) _nextPrimitive;
    }
    
    u_long *GetOt( void )
    {
        return( _orderingTable[_doubleBufferIndex] );
    }
    
    void Display( void )
    {
        VSync( 0 );
        DrawSync( 0 );
        SetDispMask( 1 );
        
        _doubleBufferIndex = !_doubleBufferIndex;
        
        PutDispEnv( &_displayEnvironment[_doubleBufferIndex] );
        PutDrawEnv( &_drawEnvironment[_doubleBufferIndex] );
        
        DrawOTag( _orderingTable[!_doubleBufferIndex]+(_orderingTableCount-1) );
        
        ClearOTagR( _orderingTable[_doubleBufferIndex], _orderingTableCount );
        _nextPrimitive = _primitives[_doubleBufferIndex];
        
    } /* Display */
    
    void LoadTextures(TIM_IMAGE& tim, u_long* texture)
    {
        GetTimInfo( texture, &tim ); /* Get TIM parameters */
        LoadImage( tim.prect, tim.paddr );        /* Upload texture to VRAM */
        if( tim.mode & 0x8 ) {
            LoadImage( tim.crect, tim.caddr );    /* Upload CLUT if present */
        }
        DrawSync(0);
    }

    inline void DrawSpriteRect(const TIM_IMAGE &tim, const SVECTOR &pos, const RECT &rect, const DVECTOR &uv, const CVECTOR &color)
    {
        SPRT *sprt = GetNextPri<SPRT>();

        setSprt(sprt);
        setXY0(sprt, pos.vx, pos.vy);
        setWH(sprt, rect.w, rect.h);
        setRGB0(sprt, color.r, color.g, color.b);
        setUV0(sprt, uv.vx, uv.vy);
        setClut(sprt, tim.crect->x, tim.crect->y);

        addPrim(GetOt(), sprt);

        IncPri<SPRT>();

        DR_TPAGE *tpri = GetNextPri<DR_TPAGE>();
        auto tpage = getTPage(tim.mode, 0, tim.prect->x, tim.prect->y);
        setDrawTPage(tpri, 0, 0, tpage);
        addPrim(GetOt(), tpri);
        IncPri<DR_TPAGE>();
    }

}; /* Graphics */

#endif //__GRAPHICS_H__