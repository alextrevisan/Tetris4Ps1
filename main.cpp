#include <psxpad.h>
#include "engine/Graphics.h"
#include "engine/Joystick.h"

Graphics *otable;

namespace
{
    int fps;
    int fps_counter;
    int fps_measure;

    void vsync_cb(void)
    {
        fps_counter++;
        if (fps_counter >= 60)
        {
            fps = fps_measure;
            fps_measure = 0;
            fps_counter = 0;
        }
    }
}

inline void DrawSpriteRect(const TIM_IMAGE &tim, const SVECTOR &pos, const RECT &rect, const DVECTOR &uv, const CVECTOR &color)
{
    SPRT *sprt = (SPRT *)otable->GetNextPri();

    setSprt(sprt);
    setXY0(sprt, pos.vx, pos.vy);
    setWH(sprt, rect.w, rect.h);
    setRGB0(sprt, color.r, color.g, color.b);
    setUV0(sprt, uv.vx, uv.vy);
    setClut(sprt, tim.crect->x, tim.crect->y);

    addPrim(otable->GetOt(), sprt);

    otable->IncPri( sizeof(SPRT) );

    DR_TPAGE *tpri = (DR_TPAGE *)otable->GetNextPri();
    auto tpage = getTPage(tim.mode, 0, tim.prect->x, tim.prect->y);
    setDrawTPage(tpri, 0, 0, tpage);
    addPrim(otable->GetOt(), tpri);
    otable->IncPri( sizeof(DR_TPAGE) );
}

inline void DrawSprite(const TIM_IMAGE &tim, const SVECTOR &pos)
{
    SPRT *sprt = (SPRT *)otable->GetNextPri();

    setSprt(sprt);
    setXY0(sprt, pos.vx, pos.vy);
    setWH(sprt, 320, 192);
    setRGB0(sprt, 128, 128, 128);
    setUV0(sprt, 0, 0);
    setClut(sprt, tim.crect->x, tim.crect->y);

    addPrim(otable->GetOt(), sprt);

    otable->IncPri( sizeof(SPRT) );

    DR_TPAGE *tpri = (DR_TPAGE *)otable->GetNextPri();
    auto tpage = getTPage(tim.mode, 0, tim.prect->x, tim.prect->y);
    setDrawTPage(tpri, 0, 0, tpage);
    addPrim(otable->GetOt() , tpri);
    otable->IncPri( sizeof(DR_TPAGE) );
}

extern u_long background[];
extern u_long tiles[];
extern u_long balls[];

TIM_IMAGE background_tim;
TIM_IMAGE tile_tim;
TIM_IMAGE ball_tim;

uint8_t pad_buff[2][34];
PADTYPE *pad;

class Tetris
{
    public:
    Tetris()
    {
        _initialized = false;
        //NewPiece();
    }
private:
    static constexpr int Width = 10;
    static constexpr int Height = 20;
    static constexpr short tileSize = 9;
    int Field[Height][Width] = {0};
    int Figures[7][4] = {
        {1,3,5,7}, //I
        {2,4,5,7}, //Z
        {3,5,4,6}, //S
        {3,5,4,7}, //T
        {2,3,5,7}, //L
        {3,5,7,6}, //J
        {2,3,4,5}, //O
    };

    struct
    {
        short x, y;
    }currentPiece[4]{{0,0},{0,0},{0,0}, {0,0}}, temp[4]{{0,0},{0,0},{0,0},{0,0}}, p{0,0};

    Joystick joystick;
    int _delayToMove = 30;
    int _frameDelayCount = 0;
    short _colorNum = 0;
    bool _initialized = false;
    int delayBeforeNewPiece = 30;
    int currentPieceDelay = 0;   

    void NewPiece()
    {
        srand(GetRCnt(0));
        _colorNum = rand()%7;
        int n = rand()%7;
        for(int i = 0; i < 4; ++i )
        {
            currentPiece[i].x = Figures[n][i] % 2 + 4;
            currentPiece[i].y = Figures[n][i] / 2 - 1;
        }
    }

    void DrawField()
    {
        for(int x = 0; x < Width; ++x)
        {
            for(int y = 0; y < Height; ++y)
            {
                if(Field[y][x] == 0)
                    continue;

                DrawSpriteRect(tile_tim, {(short)(x*tileSize + 14), (short)(y*tileSize+16)}, {0, 0, 9, 9}, {(short)((Field[y][x]-1)*9), 0}, {127, 127, 127});
            }
        }
    }

    void Draw()
    {
        DrawField();
        for(int i = 0; i < 4; ++i)
        {
            DrawSpriteRect(tile_tim, {(short)(currentPiece[i].x*tileSize + 14), (short)(currentPiece[i].y*tileSize+16)}, {0, 0, 9, 9}, {(short)(_colorNum*9), 0}, {127, 127, 127});
        }
    }

    void CopyCurrentPiece()
    {
        for(int i = 0; i < 4; ++i)
        {
            temp[i] = currentPiece[i];
        }
    }

    void RestoreCopiedPiece()
    {
        for(int i = 0; i < 4; ++i)
        {
            currentPiece[i] = temp[i];
        }
    }

    void Rotate()
    {
        CopyCurrentPiece();
        p = currentPiece[1];
        for(int i = 0; i < 4; ++i)
        {
            int x = currentPiece[i].y - p.y;
            int y = currentPiece[i].x - p.x;
            currentPiece[i].x = p.x - x;
            currentPiece[i].y = p.y + y;
        }
        if(!IsInsideBounds(0,0) && !CheckField())
        {
            RestoreCopiedPiece();
        }
    }

    bool CheckField(int deltaY = 0)
    {
        for(int i = 0; i < 4; ++i)
        {
            if(Field[currentPiece[i].y+deltaY][currentPiece[i].x] != 0)
            {
                return false;
            }
        }
        return true;
    }
    
    void Move(int x, int y)
    {
        CopyCurrentPiece();
        if(IsInsideBounds(x, y))
        {
            for(int i = 0; i < 4; ++i)
            {
                currentPiece[i].x += x;
                currentPiece[i].y += y;
            }
        }
        if(!CheckField(0))
        {
            RestoreCopiedPiece();
        }
    }

    bool IsInsideBounds(int x, int y)
    {
        for(int i = 0; i < 4; ++i)
        {
            if(currentPiece[i].x + x < 0)
                return false;
            if(currentPiece[i].x + x >= Width)
                return false;
            if(currentPiece[i].y + y >= Height)
                return false;
        }

        return true;
    }

    void UpdateInput()
    {
        if(joystick.Status() == 0)
        {
            if(joystick.JustPressed(PAD_START) && !_initialized)
            {
                _initialized = true;
                NewPiece();
            }
            if(_initialized)
            {
                if(joystick.IsPressed(PAD_DOWN))
                {
                    _frameDelayCount+=10;
                }
                if( joystick.JustPressed(PAD_UP))
                {
                    Rotate();
                }
                else if( joystick.JustPressed(PAD_LEFT) )
                {
                    Move(-1, 0);
                }
                else if( joystick.JustPressed(PAD_RIGHT) )
                {
                    Move(1, 0);
                }
            }
        }

        joystick.Update();
    }

    void MoveY()
    {
        _frameDelayCount++;
        if(_frameDelayCount > _delayToMove && _initialized)
        {
            _frameDelayCount = 0;
            Move(0, 1);
        }
    }

    bool Check()
    {
        for(int i = 0; i < 4; ++i)
        {
            if(currentPiece[i].y+1 >= Height)
            {
                return false;
            }
            if(!CheckField(1))
            {
                return false;
            }
        }
        return true;
    }

    void CheckLines()
    {
        int k = Height - 1;
        for(int y = Height -1; y > 0; --y)
        {
            int count = 0;
            for(int x = 0; x < Width; ++x)
            {
                if(Field[y][x] != 0)
                {
                    count++;
                }
                Field[k][x] = Field[y][x];
            }
            if(count < Width)
            {
                k--;
            }
        }
    }

public:
    
    void Update()
    {
        UpdateInput();
        MoveY();
        Draw();
        
        if(!Check())
        {
            currentPieceDelay++;
            if(currentPieceDelay < delayBeforeNewPiece)
            {
                return;
            }
            currentPieceDelay = 0;
            for(int i = 0; i < 4; ++i)
            {
                Field[currentPiece[i].y][currentPiece[i].x] = _colorNum + 1;
            }
            NewPiece();
        }
        CheckLines();
    }
};

int main( int argc, const char *argv[] )
{
    
    ResetGraph( 0 );
    otable = new Graphics();
    otable->SetRes( 320, 240 );
    // Init BIOS pad driver and set pad buffers (buffers are updated
	// automatically on every V-Blank)
	InitPAD(&pad_buff[0][0], 34, &pad_buff[1][0], 34);
	
	// Start pad
	StartPAD();
	
	// Don't make pad driver acknowledge V-Blank IRQ (recommended)
	ChangeClearPAD(0);

    

    otable->LoadTextures(background_tim, background);
    otable->LoadTextures(tile_tim, tiles);


    VSyncCallback(vsync_cb);
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 100);
    DrawSync(0);

    pad = (PADTYPE*)&pad_buff[0][0];

    Tetris tetris;

    while( 1 )
    {
        
        FntPrint(-1, "FPS=%d\n", fps);
        FntFlush(-1);
        fps_measure++;
        
        tetris.Update();
        DrawSpriteRect(background_tim, {0, 0}, {0, 0, 160, 240}, {0, 0}, {86, 86, 86});
        DrawSpriteRect(background_tim, {160, 0}, {0, 0, 160, 240}, {0, 0}, {127, 127, 127});
        
        otable->Display();
    }
    
    return( 0 );
}