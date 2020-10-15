/**
 *  Orginally from:
 *    https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/
 *    https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/circular_buffer.cpp
 *
 *  Made into stack instead of queue, added reading prev/next while not pop/push
 *  and no pop push while reading.
 */

#include "Scintilla.h"
#include "CircularStackLinkList.h"

#ifdef _DEBUG
#include <sstream>
extern std::wstringstream debugString;
#endif

extern NppData nppData;

#define STACK_SIZE 25

circular_buffer<tDocPos> prevPos( STACK_SIZE );
circular_buffer<tDocPos> nextPos( STACK_SIZE );

tDocPos getCurrentPos()
{
    tDocPos docPos;
    SendMessage( nppData._nppHandle, NPPM_GETFULLCURRENTPATH,
                 ( WPARAM ) MAX_PATH, ( LPARAM ) docPos.docName );

    HWND currScintilla = getCurScintilla();
    Sci_Position currentPos = ( Sci_Position )::SendMessage( currScintilla,
                              SCI_GETCURRENTPOS, 0, 0 );
    docPos.lineNo = ( int )::SendMessage( currScintilla, SCI_LINEFROMPOSITION,
                                          currentPos, 0 );

    return docPos;
}

void gotoNewPos( tDocPos docPos )
{
    ::SendMessage( nppData._nppHandle, NPPM_DOOPEN, 0,
                   ( LPARAM ) docPos.docName );

    ::SendMessage( getCurScintilla(), SCI_SETVISIBLEPOLICY, CARET_JUMPS | CARET_EVEN, 0 );
    ::SendMessage( getCurScintilla(), SCI_ENSUREVISIBLEENFORCEPOLICY, docPos.lineNo, 0 );

    ::SendMessage( getCurScintilla(), SCI_GOTOLINE, docPos.lineNo, 0 );

    ::SendMessage( getCurScintilla(), SCI_SETVISIBLEPOLICY, CARET_EVEN, 0 );
    ::SendMessage( getCurScintilla(), SCI_ENSUREVISIBLEENFORCEPOLICY, docPos.lineNo, 0 );
}

void gotoPrevPos()
{
    if ( ! prevPos.empty() )
    {
        tDocPos x;

        if ( prevPos.getTimerPut() )
        {
#ifdef _DEBUG
            OutputDebugString( TEXT("clearing TIMERPUT") );
#endif
            prevPos.clearTimerPut();
            x = prevPos.get();

            if ( prevPos.empty() )
                return;
        }
        else
            x = getCurrentPos();

#ifdef _DEBUG
        debugString << "NEXTPOS.put:" << x.docName << ":" << x.lineNo << std::endl;
        OutputDebugString( debugString.str().c_str() );
        debugString.str(TEXT("")); debugString.clear();
#endif
        nextPos.put( x );
        x = prevPos.get();
#ifdef _DEBUG
        debugString << "    (here) :" << x.docName << ":" << x.lineNo << std::endl;
        OutputDebugString( debugString.str().c_str() );
        debugString.str(TEXT("")); debugString.clear();
#endif
        gotoNewPos( x );
    }
}

void gotoNextPos()
{
    if ( ! nextPos.empty() )
    {
        tDocPos x;

        if ( ! prevPos.getTimerPut() )
        {
            x = getCurrentPos();
#ifdef _DEBUG
            debugString << "prevPos.put:" << x.docName << ":" << x.lineNo << std::endl;
            OutputDebugString( debugString.str().c_str() );
            debugString.str(TEXT("")); debugString.clear();
#endif
            prevPos.put( x );
        }

        x = nextPos.get();
#ifdef _DEBUG
        debugString << "    (here) :" << x.docName << ":" << x.lineNo << std::endl;
        OutputDebugString( debugString.str().c_str() );
        debugString.str(TEXT("")); debugString.clear();
#endif
        gotoNewPos( x );
    }
}
