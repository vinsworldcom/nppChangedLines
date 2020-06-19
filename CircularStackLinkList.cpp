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

extern NppData nppData;

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
    ::SendMessage( getCurScintilla(), SCI_GOTOLINE, docPos.lineNo, 0 );
}

void gotoPrevPos()
{
    if ( ! prevPos.empty() )
    {
        nextPos.put( getCurrentPos() );
        gotoNewPos( prevPos.get() );
    }
}

void gotoNextPos()
{
    if ( ! nextPos.empty() )
    {
        prevPos.put( getCurrentPos() );
        gotoNewPos( nextPos.get() );
    }
}
